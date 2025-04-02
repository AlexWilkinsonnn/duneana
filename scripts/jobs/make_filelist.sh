#!/bin/bash
################################################################################
# Script to make a list of xrootd paths to use in jobs.
# Run in an SL7 container with "./make_filelist.sh"
################################################################################
# Options

RUCIO_CONTAINER="fardet-hd:fardet-hd-reco2_ritm2032831_atmnu_skip0_limit10000_2073"
N=5000

################################################################################

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup rucio
setup justin
justin -v get-token

if [ ! -f job_fnames.txt ];
then
  touch job_fnames.txt
else
  echo "job_fnames.txt already exists! wont overwrite"
  exit
fi

for fname in $(rucio -a justinreadonly list-files --csv $RUCIO_CONTAINER | head -n $N | sed "s/,.*//")
do
  root_path=$(rucio -a justinreadonly list-file-replicas --protocols root --pfns $fname | grep -v "tape_backed" | head -n 1)
  echo $root_path >> job_fnames.txt
done
