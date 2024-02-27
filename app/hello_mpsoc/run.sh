#!/bin/bash
# File: run_hello_mpsoc.sh

# This script
#   - creates a board support package for the hardware platform
#   - compiles the application and generates an executable 
#   - downloads the hardware to the board
#   - starts a terminal window
#   - downloads the software and starts the application
# 
# Start the script with sh ./run.sh

CORE_DIR=../../hardware/de2_nios2_mpsoc
CORE_FILE=$CORE_DIR/nios2_mpsoc.sopcinfo
SOF_FILE=$CORE_DIR/de2_nios2_mpsoc.sof
JDI_FILE=$CORE_DIR/de2_nios2_mpsoc.jdi
BSP_PATH=../../bsp/mpsoc_hello_mpsoc
SRC_PATH=./src

APP=hello_mpsoc        # same name as the folder
CPU=cpu
NODES=4

# checking if the core or the run script has been modified, to avoid
# unnecessary recompilation of the BSP.
SOPC_BASE=$CORE_DIR/$(basename $CORE_FILE .sopcinfo)
if [[ `md5sum $SOPC_BASE.*` == `cat $CORE_DIR/.update.md5` ]] && \
       [[ `md5sum $(basename $0)` == `cat .run.md5` ]]; then 
    echo "Will not rebuild the bsp files."
    REMAKE_BSP=false
else
    echo "Will build the BSP files."
    REMAKE_BSP=true
    md5sum $md5sum $SOPC_BASE.* > $CORE_DIR/.update.md5
    md5sum $(basename $0) > .run.md5
fi


# Create BSP-package and compiling code for processor 0
if [ ! -d ${BSP_PATH}_0 ] || [ "$REMAKE_BSP" = true ]; then
    echo ""
    echo "***********************************************"
    echo "Building BSP: ${BSP_PATH}_0"
    echo "***********************************************"
    echo ""
    nios2-bsp hal ${BSP_PATH}_0 $CORE_FILE \
	      --cpu-name ${CPU}_0 \
	      --set hal.make.bsp_cflags_debug -g \
	      --set hal.make.bsp_cflags_optimization -Os \
	      --set hal.enable_small_c_library 1 \
	      --set hal.enable_reduced_device_drivers 1 \
	      --set hal.enable_lightweight_device_driver_api 1 \
	      --set hal.enable_sopc_sysid_check 1 \
	      --set hal.max_file_descriptors 4 \
	      --default_sections_mapping sram
    echo " "
    echo "BSP package creation finished"
    echo " "
fi

cd ${BSP_PATH}_0
make 3>&1 1>>log.txt 2>&1
cd ../../app/$APP

# Create Application
nios2-app-generate-makefile \
    --bsp-dir ${BSP_PATH}_0 \
    --elf-name ${APP}_0.elf \
    --src-dir ${SRC_PATH}_0/ \
    --set APP_CFLAGS_OPTIMIZATION -Os

echo "" > log.txt
echo "[Compiling code for ${CPU}_0]" > log.txt
echo "" >> log.txt

# Create ELF-file
make 3>&1 1>>log.txt 2>&1

# Create BSP-package and compiling code for the rest of the processors
for i in `seq 1 $NODES`; do
    echo "" >> log.txt
    echo "[Compiling code for ${CPU}_$i]" >> log.txt
    echo "" >> log.txt

    if [ ! -d ${BSP_PATH}_$i ] || [ "$REMAKE_BSP" = true ]; then	
	echo ""
	echo "***********************************************"
	echo "Building BSP: ${BSP_PATH}_$i"
	echo "***********************************************"
	echo ""
	nios2-bsp hal ${BSP_PATH}_$i $CORE_FILE \
		  --cpu-name ${CPU}_$i \
		  --set hal.make.bsp_cflags_debug -g \
		  --set hal.make.bsp_cflags_optimization -Os \
		  --set hal.enable_small_c_library 1 \
		  --set hal.enable_reduced_device_drivers 1 \
		  --set hal.enable_lightweight_device_driver_api 1 \
		  --set hal.enable_sopc_sysid_check 1 \
		  --set hal.max_file_descriptors 4 \
		  --default_sections_mapping onchip_$i \
		  --set hal.sys_clk_timer none \
		  --set hal.timestamp_timer none \
		  --set hal.enable_exit false \
		  --set hal.enable_c_plus_plus false \
		  --set hal.enable_clean_exit false \
		  --set hal.enable_sim_optimize false
    fi

    # Create Application
    nios2-app-generate-makefile \
	--bsp-dir ${BSP_PATH}_$i \
	--elf-name ${APP}_$i.elf \
	--src-dir ${SRC_PATH}_$i/ \
	--set APP_CFLAGS_OPTIMIZATION -Os

    # Create ELF-file
    make 3>&1 1>>log.txt 2>&1
    
done

# Download Hardware to Board

echo ""
echo "***********************************************"
echo "Download hardware to board"
echo "***********************************************"
echo ""

nios2-configure-sof $SOF_FILE

# Start Nios II Terminal for each processor

for i in `seq 0 $NODES`; do
    echo ""
    echo "Start NiosII terminal ..."
    xterm -title "${CPU}_$i" -e "nios2-terminal -i $i" &
done

for i in `seq 0 $NODES`; do
    echo ""
    echo "***********************************************"
    echo "Download software to board"
    echo "***********************************************"
    echo ""
    
    nios2-download -g ${APP}_$i.elf --cpu_name ${CPU}_$i --jdi $JDI_FILE

    echo ""
    echo "Statistics"
    nios2-elf-size ${APP}_$i.elf
done

echo ""
echo "Code compilation errors are logged in 'log.txt'"
