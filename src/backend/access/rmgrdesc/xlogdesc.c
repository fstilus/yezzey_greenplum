/*-------------------------------------------------------------------------
 *
 * xlogdesc.c
 *	  rmgr descriptor routines for access/transam/xlog.c
 *
 * Portions Copyright (c) 1996-2014, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/rmgrdesc/xlogdesc.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/xlog.h"
#include "access/xlog_internal.h"
#include "catalog/pg_control.h"
#include "utils/guc.h"
#include "utils/timestamp.h"

#include "access/twophase.h"
#include "cdb/cdbpublic.h"

/*
 * GUC support
 */
const struct config_enum_entry wal_level_options[] = {
	{"minimal", WAL_LEVEL_MINIMAL, false},
	{"archive", WAL_LEVEL_ARCHIVE, false},
	{"hot_standby", WAL_LEVEL_HOT_STANDBY, false},
	{"logical", WAL_LEVEL_LOGICAL, false},
	{NULL, 0, false}
};


/*
 * This is used also in the redo function, but must be defined here so that it
 * can also be used in xlog_desc.
 */
void
UnpackCheckPointRecord(XLogRecord *record, CheckpointExtendedRecord *ckptExtended)
{
	char *current_record_ptr;
	int remainderLen;

	if (record->xl_len == sizeof(CheckPoint))
	{
		/* Special (for bootstrap, xlog switch, maybe others) */
		ckptExtended->dtxCheckpoint = NULL;
		ckptExtended->dtxCheckpointLen = 0;
		ckptExtended->ptas = NULL;
		return;
	}

	/* Normal checkpoint Record */
	Assert(record->xl_len > sizeof(CheckPoint));

	current_record_ptr = ((char*)XLogRecGetData(record)) + sizeof(CheckPoint);
	remainderLen = record->xl_len - sizeof(CheckPoint);

	/* Start of distributed transaction information */
	ckptExtended->dtxCheckpoint = (TMGXACT_CHECKPOINT *)current_record_ptr;
	ckptExtended->dtxCheckpointLen =
		TMGXACT_CHECKPOINT_BYTES((ckptExtended->dtxCheckpoint)->committedCount);

	/*
	 * The master prepared transaction aggregate state (ptas) will be skipped
	 * when gp_before_filespace_setup is ON.
	 */
	if (remainderLen > ckptExtended->dtxCheckpointLen)
	{
		current_record_ptr = current_record_ptr + ckptExtended->dtxCheckpointLen;
		remainderLen -= ckptExtended->dtxCheckpointLen;

		/* Finally, point to prepared transaction information */
		ckptExtended->ptas = (prepared_transaction_agg_state *) current_record_ptr;
		Assert(remainderLen == PREPARED_TRANSACTION_CHECKPOINT_BYTES(ckptExtended->ptas->count));
	}
	else
	{
		Assert(remainderLen == ckptExtended->dtxCheckpointLen);
		ckptExtended->ptas = NULL;
	}
}

void
xlog_desc(StringInfo buf, XLogRecord *record)
{
	uint8		info = record->xl_info & ~XLR_INFO_MASK;
	char		*rec = XLogRecGetData(record);

	if (info == XLOG_CHECKPOINT_SHUTDOWN ||
		info == XLOG_CHECKPOINT_ONLINE)
	{
		CheckPoint *checkpoint = (CheckPoint *) rec;

		CheckpointExtendedRecord ckptExtended;

		appendStringInfo(buf, "checkpoint: redo %X/%X; "
						 "tli %u; prev tli %u; fpw %s; xid %u/%u; oid %u; relfilenode %u; multi %u; offset %u; "
						 "oldest xid %u in DB %u; oldest multi %u in DB %u; "
						 "oldest running xid %u; %s",
				(uint32) (checkpoint->redo >> 32), (uint32) checkpoint->redo,
						 checkpoint->ThisTimeLineID,
						 checkpoint->PrevTimeLineID,
						 checkpoint->fullPageWrites ? "true" : "false",
						 checkpoint->nextXidEpoch, checkpoint->nextXid,
						 checkpoint->nextOid,
						 checkpoint->nextRelfilenode,
						 checkpoint->nextMulti,
						 checkpoint->nextMultiOffset,
						 checkpoint->oldestXid,
						 checkpoint->oldestXidDB,
						 checkpoint->oldestMulti,
						 checkpoint->oldestMultiDB,
						 checkpoint->oldestActiveXid,
				 (info == XLOG_CHECKPOINT_SHUTDOWN) ? "shutdown" : "online");

		UnpackCheckPointRecord(record, &ckptExtended);

		if (ckptExtended.dtxCheckpointLen > 0)
		{
			appendStringInfo(buf,
				 ", checkpoint record data length = %u, DTX committed count %d, DTX data length %u",
							 record->xl_len,
							 ckptExtended.dtxCheckpoint->committedCount,
							 ckptExtended.dtxCheckpointLen);
			if (ckptExtended.ptas != NULL)
				appendStringInfo(buf,
								 ", prepared transaction agg state count = %d",
								 ckptExtended.ptas->count);
		}
	}
	else if (info == XLOG_NOOP)
	{
		appendStringInfoString(buf, "xlog no-op");
	}
	else if (info == XLOG_NEXTOID)
	{
		Oid			nextOid;

		memcpy(&nextOid, rec, sizeof(Oid));
		appendStringInfo(buf, "nextOid: %u", nextOid);
	}
	else if (info == XLOG_NEXTRELFILENODE)
	{
		Oid			nextRelfilenode;

		memcpy(&nextRelfilenode, rec, sizeof(Oid));
		appendStringInfo(buf, "nextRelfilenode: %u", nextRelfilenode);
	}
	else if (info == XLOG_SWITCH)
	{
		appendStringInfoString(buf, "xlog switch");
	}
	else if (info == XLOG_RESTORE_POINT)
	{
		xl_restore_point *xlrec = (xl_restore_point *) rec;

		appendStringInfo(buf, "restore point: %s", xlrec->rp_name);

	}
	else if (info == XLOG_FPI)
	{
		BkpBlock   *bkp = (BkpBlock *) rec;

		appendStringInfo(buf, "full-page image: %s block %u",
						 relpathperm(bkp->node, bkp->fork),
						 bkp->block);
	}
	else if (info == XLOG_BACKUP_END)
	{
		XLogRecPtr	startpoint;

		memcpy(&startpoint, rec, sizeof(XLogRecPtr));
		appendStringInfo(buf, "backup end: %X/%X",
						 (uint32) (startpoint >> 32), (uint32) startpoint);
	}
	else if (info == XLOG_PARAMETER_CHANGE)
	{
		xl_parameter_change xlrec;
		const char *wal_level_str;
		const struct config_enum_entry *entry;

		memcpy(&xlrec, rec, sizeof(xl_parameter_change));

		/* Find a string representation for wal_level */
		wal_level_str = "?";
		for (entry = wal_level_options; entry->name; entry++)
		{
			if (entry->val == xlrec.wal_level)
			{
				wal_level_str = entry->name;
				break;
			}
		}

		appendStringInfo(buf, "parameter change: max_connections=%d max_worker_processes=%d "
						 "max_prepared_xacts=%d max_locks_per_xact=%d "
						 "wal_level=%s wal_log_hints=%s",
						 xlrec.MaxConnections,
						 xlrec.max_worker_processes,
						 xlrec.max_prepared_xacts,
						 xlrec.max_locks_per_xact,
						 wal_level_str,
						 xlrec.wal_log_hints ? "on" : "off");
	}
	else if (info == XLOG_FPW_CHANGE)
	{
		bool		fpw;

		memcpy(&fpw, rec, sizeof(bool));
		appendStringInfo(buf, "full_page_writes: %s", fpw ? "true" : "false");
	}
	else if (info == XLOG_END_OF_RECOVERY)
	{
		xl_end_of_recovery xlrec;

		memcpy(&xlrec, rec, sizeof(xl_end_of_recovery));
		appendStringInfo(buf, "end_of_recovery: tli %u; prev tli %u; time %s",
						 xlrec.ThisTimeLineID, xlrec.PrevTimeLineID,
						 timestamptz_to_str(xlrec.end_time));
	}
	else if (info == XLOG_OVERWRITE_CONTRECORD)
	{
		xl_overwrite_contrecord xlrec;

		memcpy(&xlrec, rec, sizeof(xl_overwrite_contrecord));
		appendStringInfo(buf, "lsn %X/%X; time %s",
						 (uint32) (xlrec.overwritten_lsn >> 32),
						 (uint32) xlrec.overwritten_lsn,
						 timestamptz_to_str(xlrec.overwrite_time));
	}
	else
		appendStringInfoString(buf, "UNKNOWN");
}
