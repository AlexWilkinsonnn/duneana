#!/bin/bash
################################################################################
# Script to run pandora reco1 fcl with vertex monitoring turned on over the
# atmospheric neutrino rucio dataset
################################################################################
# Options

RUCIO_CONTAINER="fardet-hd:fardet-hd-reco2_ritm2032831_atmnu_skip0_limit10000_2073"
OUTPUT_DIR="/pnfs/dune/scratch/users/awilkins/atms_vtx_bias/root_outputs"
RECO2_FCL="standard_pandoravtxmonitoring_atmos_dune10kt_1x2x6.fcl"
# RECO2_FCL="standard_pandoramanyvtxmonitoring_atmos_dune10kt_1x2x6.fcl"
JOB_FNAMES_REL_PATH="/srcs/duneana/scripts/jobs/job_fnames.txt"

################################################################################

# Don't try over and over again to copy a file when it isn't going to work
export IFDH_CP_UNLINK_ON_ERROR=1
export IFDH_CP_MAXRETRIES=1

echo "Running on $(hostname) at ${GLIDEIN_Site}. GLIDEIN_DUNESite = ${GLIDEIN_DUNESite}. At ${PWD}"
echo "I am $USER"

# Setup env
${INPUT_TAR_DIR_LOCAL}/srcs/duneana/scripts/jobs/make_setup_grid.sh ${INPUT_TAR_DIR_LOCAL}/localProducts_larsoft_*/setup \
                                                                    setup-grid

input_root_path=$(cat ${INPUT_TAR_DIR_LOCAL}/${JOB_FNAMES_REL_PATH} | head -n $((PROCESS+1)) | tail -n -1)

# only the very newest shiniest xrootd works with tokens... wtf. Solution is a wasteful copy which will only work with the shiniest ifdhc...
source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup ifdhc v2_8_0
ifdh cp -D ${input_root_path} .
input_name=$(basename $input_root_path)
ls -lrth
unsetup_all
echo "Input file is ${input_name} from ${input_root_path}"

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
source setup-grid
mrbslp

# jobsub stopped using proxies + rucio doesnt accept tokens -> using justinreadonly only work interactively -> this does not work
# Solution is to wait for rucio to work with proxies and use the caveman 'make_filelist.sh' approach for now.
# setup rucio
# setup justin
# justin -v get-token
# input_fname=$(rucio -a justinreadonly list-files --csv $RUCIO_CONTAINER | head -n $((PROCESS+1)) | tail -n -1 | sed "s/,.*//")
# input_root_path=$(rucio -a justinreadonly list-file-replicas --protocols root --pfns $input_fname | grep -v "tape_backed" | head -n 1)

# if [ -z "$input_root_path" ];
# then
#   echo "No staged file anywhere for $input_fname"
# fi
# echo "input_file is ${input_fname}"
# echo "Accessing it with ${input_root_path}"

lar -c $RECO2_FCL -s $input_name -n -1

ls -lrth

echo "Copying output pandora validation file to dCache..."
ifdh cp PandoraVertexMonitoring.root ${OUTPUT_DIR}/PandoraVertexMonitoring_${PROCESS}.root

