## For CentOS:

- Install Dependencies

  ```bash
  ./README.CentOS.bash
  ```

- If you want to link cmake3 to cmake, run:

  ```bash
  sudo ln -sf /usr/bin/cmake3 /usr/local/bin/cmake
  ```

- Make sure that you add `/usr/local/lib` and `/usr/local/lib64` to
`/etc/ld.so.conf`, then run command `ldconfig`.

- If you want to install and use gcc-6 by default, run:

  ```bash
  sudo yum install -y centos-release-scl
  sudo yum install -y devtoolset-6-toolchain
  echo 'source scl_source enable devtoolset-6' >> ~/.bashrc
  ```

## For RHEL:

- Install Development Tools.
  - For RHEL 8: Install `Development Tools`:

    ```bash
    sudo yum group install -y "Development Tools"
    ```

  - For RHEL versions (< 8.0): Install `devtoolset-7`:

    ```bash
    sudo yum-config-manager --enable rhui-REGION-rhel-server-rhscl
    sudo yum install -y devtoolset-7-toolchain
    ```

- Install dependencies using README.CentOS.bash script.
  - For RHEL 8: Execute additional steps before running README.CentOS.bash script.

    Note: Make sure installation of `Development Tools` includes `git` and `make` else install these tools manually.

    ```bash
    sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
    sed -i -e 's/python-devel /python2-devel /' -e 's/python-pip/python2-pip/' -e 's/sudo pip/sudo pip2/' README.CentOS.bash
    sed -i '/xerces-c-devel/d' README.CentOS.bash
    sudo ln -s /usr/bin/python2.7 /usr/bin/python
    ```

  - Install dependencies using README.CentOS.bash script.

    ```bash
    ./README.CentOS.bash
    ```

## For Ubuntu:

- Install Dependencies
  When you run the README.ubuntu.bash script for dependencies, you will be asked to configure realm for kerberos.
  You can enter any realm, since this is just for testing, and during testing, it will reconfigure a local server/client.
  If you want to skip this manual configuration, use:
  `export DEBIAN_FRONTEND=noninteractive`

  ```bash
  ./README.ubuntu.bash
  ```

- If you want to use gcc-6 and g++-6:

  ```bash
  add-apt-repository ppa:ubuntu-toolchain-r/test -y
  apt-get update
  apt-get install -y gcc-6 g++-6
  ```

## Common Platform Tasks:

Make sure that you add `/usr/local/lib` to `/etc/ld.so.conf`,
then run command `ldconfig`.

1. Create gpadmin and setup ssh keys

    Either use:

    ```bash
    # Requires gpdb clone to be named gpdb_src
    gpdb_src/concourse/scripts/setup_gpadmin_user.bash
    ```
    to create the gpadmin user and set up keys,

    OR

    manually create ssh keys so you can do ssh localhost without a password, e.g., 
   
    ```bash
    ssh-keygen
    cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
    chmod 600 ~/.ssh/authorized_keys
    ```

2. Verify that you can ssh to your machine name without a password

    ```bash
    ssh <hostname of your machine>  # e.g., ssh briarwood
    ```

3. Set up your system configuration:

    ```bash
    sudo bash -c 'cat >> /etc/sysctl.conf <<-EOF
    kernel.shmmax = 500000000
    kernel.shmmni = 4096
    kernel.shmall = 4000000000
    kernel.sem = 500 1024000 200 4096
    kernel.sysrq = 1
    kernel.core_uses_pid = 1
    kernel.msgmnb = 65536
    kernel.msgmax = 65536
    kernel.msgmni = 2048
    net.ipv4.tcp_syncookies = 1
    net.ipv4.ip_forward = 0
    net.ipv4.conf.default.accept_source_route = 0
    net.ipv4.tcp_tw_recycle = 1
    net.ipv4.tcp_max_syn_backlog = 4096
    net.ipv4.conf.all.arp_filter = 1
    net.ipv4.ip_local_port_range = 1025 65535
    net.core.netdev_max_backlog = 10000
    net.core.rmem_max = 2097152
    net.core.wmem_max = 2097152
    vm.overcommit_memory = 2

    EOF'

    sudo bash -c 'cat >> /etc/security/limits.conf <<-EOF
    * soft nofile 65536
    * hard nofile 65536
    * soft nproc 131072
    * hard nproc 131072

    EOF'

    sudo bash -c 'cat >> /etc/ld.so.conf <<-EOF
    /usr/local/lib

    EOF'
    ```
