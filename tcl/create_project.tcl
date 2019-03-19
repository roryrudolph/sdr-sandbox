set PROJ_DIR ../proj
set HDL_DIR  ../hdl
set XDC_DIR  ../xdc

#start_gui
create_project sandbox ${PROJ_DIR} -part xc7z020clg484-1
set_property board_part xilinx.com:zc702:part0:1.4 [current_project]
set_property target_language VHDL [current_project]

add_files -norecurse ${HDL_DIR}/top.vhd
add_files -norecurse ${HDL_DIR}/wfmlut.vhd
update_compile_order -fileset sources_1

add_files -fileset constrs_1 -norecurse ${XDC_DIR}/Zybo-Z7-Master.xdc

set_property SOURCE_SET {} [get_filesets sim_1]
add_files -fileset sim_1 -norecurse ${HDL_DIR}/wfmlut_tb.vhd ${HDL_DIR}/wfmlut.vhd
update_compile_order -fileset sim_1

