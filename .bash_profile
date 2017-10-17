# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
	. ~/.bashrc
fi

# User specific environment and startup programs

export PATH=$PATH:$HOME/bin:.

# specific to MPICH2
#PATH=$PATH:/usr/local/mpich2/bin
export PATH=/usr/apps/mpich121-`uname -p`/bin:$PATH

# specific to MapReduce
export HADOOP_HOME=$HOME/hadoop-2.4.0
export HADOOP_VERSION=2.4.0
export PATH=$PATH:$HADOOP_HOME/bin:$HADOOP_HOME/sbin

unset USERNAME
