#!/bin/bash
################################################################################
# Script to run the vertex bias analysis module over a bunch of files from the
# atmospheric neutrino rucio dataset
################################################################################
# Options

RUCIO_CONTAINER="fardet-hd:fardet-hd-reco2_ritm2032831_atmnu_skip0_limit10000_2073"
OUTPUT_DIR="/pnfs/dune/scratch/users/awilkins/atms_vtx_bias/root_outputs"

################################################################################

echo "Running on $(hostname) at ${GLIDEIN_Site}. GLIDEIN_DUNESite = ${GLIDEIN_DUNESite}. At ${PWD}"
echo "I am $USER"

# Setup env
${INPUT_TAR_DIR_LOCAL}/srcs/duneana/scripts/jobs/make_setup_grid.sh ${INPUT_TAR_DIR_LOCAL}/localProducts_larsoft_*/setup \
                                                                    setup-grid

ls -lrth

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
source setup-grid
mrbslp

# Don't try over and over again to copy a file when it isn't going to work
export IFDH_CP_UNLINK_ON_ERROR=1
export IFDH_CP_MAXRETRIES=1
export IFDH_DEBUG=0

input_fname=$(rucio -a awilkins --csv $RUCIO_CONTAINER | head -n $((PROCESS+1)) | tail -n -1 | sed "s/,.*//")
input_root_path=$(rucio -a awilkins list-file-replicas --protocols root --pfns $input_fname | grep -v "tape_backed" | head -n 1)
if [ -z "${VAR}" ];
then
  echo "No staged file anywhere for $input_fname"
fi
echo "input_file is ${input_fname}"
echo "Accessing it with ${input_root_path}"

lar -c run_VtxBiasAna.fcl -s $input_root_path -n -1

ls -lrth

echo "Copying output file to dCache..."
ifdh cp VtxBiasAnaOutput.root ${OUTPUT_DIR}/VtxBiasAnaOutput_${PROCESS}.root

