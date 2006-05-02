ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pRange/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

MOREFLAGS = -DQUIET -D_OPTIONAL_SORT_ -DPARALLEL -D_PGRAPH -DDYNAMIC_GRAPH -DEFFICIENCY

test: all

test-debug: MOREFLAGS += -g -DSTAPL_DEBUG
test-debug: all

all: compile run

# 21jan2005/timmie - These tests were removed from the compile and run sections
#                    because they try to use a method not yet defined in the 
#                    pContainers or they expose the problem of the pRange not
#                    being infinitely expandable.  They will be added back in 
#                    when the pRange is fixed and a method to find the version
#                    of a pContainer has been implemented.
#
#          prange_get_cont_vers_neg prange_get_cont_vers_pos
#          linear_prange_get_cont_vers_neg linear_prange_get_cont_vers_pos
#          prange_pcont_delete_element_neg prange_pcont_delete_element_pos
#          prange_pcont_add_element_neg prange_pcont_add_element_pos
#          prange_pcont_add_elements_neg prange_pcont_add_elements_pos
#          linear_prange_pcont_delete_element_neg
#          linear_prange_pcont_delete_element_pos
#          linear_prange_pcont_add_element_neg
#          linear_prange_pcont_add_element_pos
#          linear_prange_pcont_add_elements_neg
#          linear_prange_pcont_add_elements_pos
#          linear_prange_pcont_front_neg linear_prange_pcont_front_pos
#          linear_prange_pcont_back_neg linear_prange_pcont_back_pos
#          linear_prange_pcont_init_bndry_info_neg
#          linear_prange_pcont_init_bndry_info_pos

compile : counted_bndry_at_start_neg counted_bndry_at_start_pos \
          counted_bndry_at_finish_neg counted_bndry_at_finish_pos \
          counted_bndry_start_accessor_neg counted_bndry_start_accessor_pos \
          counted_bndry_finish_accessor_neg  counted_bndry_finish_accessor_pos \
          counted_bndry_start_modifier_neg counted_bndry_start_modifier_pos \
          counted_bndry_finish_modifier_neg counted_bndry_finish_modifier_pos \
          counted_bndry_size_neg counted_bndry_size_pos \
          graph_bndry_at_start_neg graph_bndry_at_start_pos \
          graph_bndry_at_finish_neg graph_bndry_at_finish_pos \
          graph_bndry_start_accessor_neg graph_bndry_start_accessor_pos \
          graph_bndry_finish_accessor_neg graph_bndry_finish_accessor_pos \
          graph_bndry_size_pos graph_bndry_size_neg \
          linear_bndry_at_start_neg linear_bndry_at_start_pos \
          linear_bndry_at_finish_neg linear_bndry_at_finish_pos \
          linear_bndry_start_accessor_neg linear_bndry_start_accessor_pos \
          linear_bndry_finish_accessor_neg linear_bndry_finish_accessor_pos \
          linear_bndry_start_modifier_neg linear_bndry_start_modifier_pos \
          linear_bndry_finish_modifier_neg linear_bndry_finish_modifier_pos \
          linear_bndry_size_pos linear_bndry_size_neg \
          ddg_add_task_neg ddg_add_task_pos \
          ddg_remove_task_neg ddg_remove_task_pos \
          ddg_add_dependence_neg ddg_add_dependence_pos \
          ddg_remove_dependence_neg ddg_remove_dependence_pos \
          ddg_get_ready_neg ddg_get_ready_pos \
          ddg_processed_neg ddg_processed_pos \
          ddg_finished_neg ddg_finished_pos \
          ddg_reset_pos \
          pddg_add_task_neg pddg_add_task_pos \
          pddg_remove_task_neg pddg_remove_task_pos \
          pddg_add_dependence_neg pddg_add_dependence_pos \
          pddg_remove_dependence_neg pddg_remove_dependence_pos \
          pddg_get_ready_neg pddg_get_ready_pos \
          pddg_processed_neg pddg_processed_pos \
          pddg_finished_neg pddg_finished_pos \
          pddg_reset_pos \
          prange_set_ddg_neg prange_set_ddg_pos \
          prange_get_ddg_neg prange_get_ddg_pos \
          prange_set_elem_ddg_neg prange_set_elem_ddg_pos \
          prange_get_elem_ddg_neg prange_get_elem_ddg_pos \
          linear_prange_set_ddg_neg linear_prange_set_ddg_pos \
          linear_prange_get_ddg_neg linear_prange_get_ddg_pos \
          linear_prange_set_elem_ddg_neg linear_prange_set_elem_ddg_pos \
          linear_prange_get_elem_ddg_neg linear_prange_get_elem_ddg_pos \
          prange_cnstrctr_default_neg prange_cnstrctr_default_pos \
          prange_cnstrctr_bndry_neg prange_cnstrctr_bndry_pos \
          prange_cnstrctr_cntnr_bndry_neg prange_cnstrctr_cntnr_bndry_pos \
          prange_cnstrctr_cntnr_neg prange_cnstrctr_cntnr_pos \
          prange_cnstrctr_gid_bndry_neg prange_cnstrctr_gid_bndry_pos \
          prange_cnstrctr_gid_cntnr_bndry_neg \
          prange_cnstrctr_gid_cntnr_bndry_pos \
          prange_cnstrctr_copy_neg prange_cnstrctr_copy_pos \
          prange_assignment_oper_neg prange_assignment_oper_pos \
          prange_local_size_neg prange_local_size_pos \
          prange_is_invalid_neg prange_is_invalid_pos \
          prange_get_global_id_neg prange_get_global_id_pos \
          prange_get_gid_prod_neg prange_get_gid_prod_pos \
          prange_get_thread_id_neg prange_get_thread_id_pos \
          prange_get_handle_neg prange_get_handle_pos \
          prange_get_boundary_neg prange_get_boundary_pos \
          prange_get_subranges_neg prange_get_subranges_pos \
          prange_get_subrange_neg prange_get_subrange_pos \
          prange_get_num_subranges_neg prange_get_num_subranges_pos \
          prange_set_thread_id_neg prange_set_thread_id_pos \
          prange_set_bndry_neg prange_set_bndry_pos \
          prange_add_subrange_neg prange_add_subrange_pos \
          prange_pcont_container_size_neg prange_pcont_container_size_pos \
          prange_pcont_container_local_size_neg \
          prange_pcont_container_local_size_pos \
          prange_pcont_empty_neg prange_pcont_empty_pos \
          prange_pcont_local_empty_neg prange_pcont_local_empty_pos \
          prange_pcont_clear_pos \
          prange_pcont_local_clear_pos \
          prange_pcont_local_begin_neg prange_pcont_local_begin_pos \
          prange_pcont_local_end_neg prange_pcont_local_end_pos \
          prange_pcont_local_rbegin_neg prange_pcont_local_rbegin_pos \
          prange_pcont_local_rend_neg prange_pcont_local_rend_pos \
          prange_pcont_get_element_neg prange_pcont_get_element_pos \
          prange_pcont_set_element_neg prange_pcont_set_element_pos \
          prange_pcont_update_element_neg prange_pcont_update_element_pos \
          prange_pcont_is_local_neg prange_pcont_is_local_pos \
          prange_pcont_lookup_neg prange_pcont_lookup_pos \
          prange_pcont_find_remote_pid_neg prange_pcont_find_remote_pid_pos \
          prange_pcont_complete_search_neg prange_pcont_complete_search_pos \
          linear_prange_cnstrctr_default_neg \
          linear_prange_cnstrctr_default_pos \
          linear_prange_cnstrctr_bndry_neg linear_prange_cnstrctr_bndry_pos \
          linear_prange_cnstrctr_cntnr_bndry_neg \
          linear_prange_cnstrctr_cntnr_bndry_pos \
          linear_prange_cnstrctr_cntnr_neg linear_prange_cnstrctr_cntnr_pos \
          linear_prange_cnstrctr_gid_bndry_neg \
          linear_prange_cnstrctr_gid_bndry_pos \
          linear_prange_cnstrctr_gid_cntnr_bndry_neg \
          linear_prange_cnstrctr_gid_cntnr_bndry_pos \
          linear_prange_cnstrctr_copy_neg linear_prange_cnstrctr_copy_pos \
          linear_prange_assignment_oper_neg linear_prange_assignment_oper_pos \
          linear_prange_local_size_neg linear_prange_local_size_pos \
          linear_prange_is_invalid_neg linear_prange_is_invalid_pos \
          linear_prange_get_global_id_neg linear_prange_get_global_id_pos \
          linear_prange_get_gid_prod_neg linear_prange_get_gid_prod_pos \
          linear_prange_get_thread_id_neg linear_prange_get_thread_id_pos \
          linear_prange_get_handle_neg linear_prange_get_handle_pos \
          linear_prange_get_boundary_neg linear_prange_get_boundary_pos \
          linear_prange_get_subranges_neg linear_prange_get_subranges_pos \
          linear_prange_get_subrange_neg linear_prange_get_subrange_pos \
          linear_prange_get_num_subranges_neg \
          linear_prange_get_num_subranges_pos \
          linear_prange_set_thread_id_neg linear_prange_set_thread_id_pos \
          linear_prange_set_bndry_neg linear_prange_set_bndry_pos \
          linear_prange_add_subrange_neg linear_prange_add_subrange_pos \
          linear_prange_pcont_container_size_neg \
          linear_prange_pcont_container_size_pos \
          linear_prange_pcont_container_local_size_neg \
          linear_prange_pcont_container_local_size_pos \
          linear_prange_pcont_empty_neg linear_prange_pcont_empty_pos \
          linear_prange_pcont_local_empty_neg \
          linear_prange_pcont_local_empty_pos \
          linear_prange_pcont_clear_pos \
          linear_prange_pcont_local_clear_pos \
          linear_prange_pcont_local_begin_neg \
          linear_prange_pcont_local_begin_pos \
          linear_prange_pcont_local_end_neg linear_prange_pcont_local_end_pos \
          linear_prange_pcont_local_rbegin_neg \
          linear_prange_pcont_local_rbegin_pos \
          linear_prange_pcont_local_rend_neg \
          linear_prange_pcont_local_rend_pos \
          linear_prange_pcont_get_element_neg \
          linear_prange_pcont_get_element_pos \
          linear_prange_pcont_set_element_neg \
          linear_prange_pcont_set_element_pos \
          linear_prange_pcont_update_element_neg \
          linear_prange_pcont_update_element_pos \
          linear_prange_pcont_is_local_neg linear_prange_pcont_is_local_pos \
          linear_prange_pcont_lookup_neg linear_prange_pcont_lookup_pos \
          linear_prange_pcont_find_remote_pid_neg \
          linear_prange_pcont_find_remote_pid_pos \
          linear_prange_pcont_complete_search_neg \
          linear_prange_pcont_complete_search_pos \
          linear_prange_pcont_max_size_neg linear_prange_pcont_max_size_pos \
          linear_prange_pcont_insert_neg linear_prange_pcont_insert_pos \
          linear_prange_pcont_erase_neg linear_prange_pcont_erase_pos \
          linear_prange_pcont_push_back_neg linear_prange_pcont_push_back_pos \
          linear_prange_pcont_pop_back_pos \
          pinner psum pforall pforeach


# Tests removed from run
#
# 21jan2005/timmie - These tests were removed from the run section because they
#                    use functionality not present in the pRange.  They will be
#                    added back in as the pRange is upgraded.
#
#     prange_pcont_clear prange_pcont_local_clear
#     linear_prange_pcont_clear linear_prange_pcont_local_clear
#     prange_set_bndry linear_prange_set_bndry
#     prange_local_size linear_prange_local_size
#
#
# 21jan2005/timmie - These tests were removed from the compile and run sections
#                    because they try to use a method not yet defined in the 
#                    pContainers or they expose the problem of the pRange not
#                    being infinitely expandable.  They will be added back in 
#                    when the pRange is fixed and a method to find the version
#                    of a pContainer has been implemented.
#
#     prange_get_cont_vers linear_prange_get_cont_vers
#     prange_add_subrange linear_prange_add_subrange
#     prange_pcont_delete_element linear_prange_pcont_delete_element
#     prange_pcont_add_element prange_pcont_add_elements
#     linear_prange_pcont_add_element linear_prange_pcont_add_elements
#     linear_prange_pcont_max_size linear_prange_pcont_front
#     linear_prange_pcont_back linear_prange_pcont_insert
#     linear_prange_pcont_erase linear_prange_pcont_push_back
#     linear_prange_pcont_pop_back linear_prange_pcont_init_bndry_info

run: counted_bndry_at_start counted_bndry_at_finish \
     counted_bndry_start_accessor counted_bndry_finish_accessor \
     counted_bndry_start_modifier counted_bndry_finish_modifier \
     counted_bndry_size \
     graph_bndry_at_start graph_bndry_at_finish \
     graph_bndry_start_accessor graph_bndry_finish_accessor graph_bndry_size \
     linear_bndry_at_start linear_bndry_at_finish \
     linear_bndry_start_accessor linear_bndry_finish_accessor \
     linear_bndry_start_modifier linear_bndry_finish_modifier \
     linear_bndry_size \
     ddg_add_task ddg_remove_task ddg_add_dependence ddg_remove_dependence \
     ddg_get_ready ddg_processed ddg_finished ddg_reset \
     pddg_add_task pddg_remove_task pddg_add_dependence pddg_remove_dependence \
     pddg_get_ready pddg_processed pddg_finished pddg_reset \
     prange_set_ddg prange_get_ddg \
     prange_set_elem_ddg prange_get_elem_ddg \
     linear_prange_set_ddg linear_prange_get_ddg \
     linear_prange_set_elem_ddg linear_prange_get_elem_ddg \
     prange_cnstrctr_default prange_cnstrctr_bndry \
     prange_cnstrctr_cntnr_bndry prange_cnstrctr_cntnr \
     prange_cnstrctr_gid_bndry prange_cnstrctr_gid_cntnr_bndry \
     prange_cnstrctr_copy prange_assignment_oper \
     prange_is_invalid \
     prange_get_global_id prange_get_gid_prod \
     prange_get_thread_id prange_get_handle prange_get_boundary \
     prange_get_subranges prange_get_subrange prange_get_num_subranges \
     prange_set_thread_id \
     prange_pcont_container_size prange_pcont_container_local_size \
     prange_pcont_empty prange_pcont_local_empty \
     prange_pcont_local_begin prange_pcont_local_end \
     prange_pcont_local_rbegin prange_pcont_local_rend \
     prange_pcont_get_element prange_pcont_set_element \
     prange_pcont_update_element \
     prange_pcont_is_local prange_pcont_lookup \
     prange_pcont_find_remote_pid prange_pcont_complete_search \
     linear_prange_cnstrctr_default linear_prange_cnstrctr_bndry \
     linear_prange_cnstrctr_cntnr_bndry linear_prange_cnstrctr_cntnr \
     linear_prange_cnstrctr_gid_bndry linear_prange_cnstrctr_gid_cntnr_bndry \
     linear_prange_cnstrctr_copy linear_prange_assignment_oper \
     linear_prange_is_invalid \
     linear_prange_get_global_id \
     linear_prange_get_gid_prod linear_prange_get_thread_id \
     linear_prange_get_handle linear_prange_get_boundary \
     linear_prange_get_subranges linear_prange_get_subrange  \
     linear_prange_get_num_subranges \
     linear_prange_set_thread_id \
     linear_prange_pcont_container_size  \
     linear_prange_pcont_container_local_size \
     linear_prange_pcont_empty linear_prange_pcont_local_empty \
     linear_prange_pcont_local_begin linear_prange_pcont_local_end \
     linear_prange_pcont_local_rbegin linear_prange_pcont_local_rend \
     linear_prange_pcont_get_element linear_prange_pcont_set_element \
     linear_prange_pcont_update_element \
     linear_prange_pcont_is_local linear_prange_pcont_lookup \
     linear_prange_pcont_find_remote_pid linear_prange_pcont_complete_search \
     test_pinner test_psum test_pforall test_pforeach

clean : 
	rm -f \
	counted_bndry_at_start_neg counted_bndry_at_start_pos \
	counted_bndry_at_finish_neg counted_bndry_at_finish_pos \
	counted_bndry_start_accessor_neg counted_bndry_start_accessor_pos \
	counted_bndry_finish_accessor_neg  counted_bndry_finish_accessor_pos \
	counted_bndry_start_modifier_neg counted_bndry_start_modifier_pos \
	counted_bndry_finish_modifier_neg counted_bndry_finish_modifier_pos \
	counted_bndry_size_neg counted_bndry_size_pos \
	graph_bndry_at_start_neg graph_bndry_at_start_pos \
	graph_bndry_at_finish_neg graph_bndry_at_finish_pos \
	graph_bndry_start_accessor_neg graph_bndry_start_accessor_pos \
	graph_bndry_finish_accessor_neg graph_bndry_finish_accessor_pos \
	graph_bndry_size_pos graph_bndry_size_neg \
	linear_bndry_at_start_neg linear_bndry_at_start_pos \
	linear_bndry_at_finish_neg linear_bndry_at_finish_pos \
	linear_bndry_start_accessor_neg linear_bndry_start_accessor_pos \
	linear_bndry_finish_accessor_neg linear_bndry_finish_accessor_pos \
	linear_bndry_start_modifier_neg linear_bndry_start_modifier_pos \
	linear_bndry_finish_modifier_neg linear_bndry_finish_modifier_pos \
	linear_bndry_size_pos linear_bndry_size_neg \
	ddg_add_task_neg ddg_add_task_pos \
	ddg_remove_task_neg ddg_remove_task_pos \
	ddg_add_dependence_neg ddg_add_dependence_pos \
	ddg_remove_dependence_neg ddg_remove_dependence_pos \
	ddg_get_ready_neg ddg_get_ready_pos \
	ddg_processed_neg ddg_processed_pos \
	ddg_finished_neg ddg_finished_pos \
	ddg_reset_pos \
	pddg_add_task_neg pddg_add_task_pos \
	pddg_remove_task_neg pddg_remove_task_pos \
	pddg_add_dependence_neg pddg_add_dependence_pos \
	pddg_remove_dependence_neg pddg_remove_dependence_pos \
	pddg_get_ready_neg pddg_get_ready_pos \
	pddg_processed_neg pddg_processed_pos \
	pddg_finished_neg pddg_finished_pos \
	pddg_reset_pos \
	prange_set_ddg_neg prange_set_ddg_pos \
	prange_get_ddg_neg prange_get_ddg_pos \
	prange_set_elem_ddg_neg prange_set_elem_ddg_pos \
	prange_get_elem_ddg_neg prange_get_elem_ddg_pos \
	linear_prange_set_ddg_neg linear_prange_set_ddg_pos \
	linear_prange_get_ddg_neg linear_prange_get_ddg_pos \
	linear_prange_set_elem_ddg_neg linear_prange_set_elem_ddg_pos \
	linear_prange_get_elem_ddg_neg linear_prange_get_elem_ddg_pos \
	prange_cnstrctr_default_neg prange_cnstrctr_default_pos \
	prange_cnstrctr_bndry_neg prange_cnstrctr_bndry_pos \
	prange_cnstrctr_cntnr_bndry_neg prange_cnstrctr_cntnr_bndry_pos \
	prange_cnstrctr_cntnr_neg prange_cnstrctr_cntnr_pos \
	prange_cnstrctr_gid_bndry_neg prange_cnstrctr_gid_bndry_pos \
	prange_cnstrctr_gid_cntnr_bndry_neg \
	prange_cnstrctr_gid_cntnr_bndry_pos \
	prange_cnstrctr_copy_neg prange_cnstrctr_copy_pos \
	prange_assignment_oper_neg prange_assignment_oper_pos \
	prange_local_size_neg prange_local_size_pos \
	prange_is_invalid_neg prange_is_invalid_pos \
	prange_get_cont_vers_neg prange_get_cont_vers_pos \
	prange_get_global_id_neg prange_get_global_id_pos \
	prange_get_gid_prod_neg prange_get_gid_prod_pos \
	prange_get_thread_id_neg prange_get_thread_id_pos \
	prange_get_handle_neg prange_get_handle_pos \
	prange_get_boundary_neg prange_get_boundary_pos \
	prange_get_subranges_neg prange_get_subranges_pos \
	prange_get_subrange_neg prange_get_subrange_pos \
	prange_get_num_subranges_neg prange_get_num_subranges_pos \
	prange_set_thread_id_neg prange_set_thread_id_pos \
	prange_set_bndry_neg prange_set_bndry_pos \
	prange_add_subrange_neg prange_add_subrange_pos \
	prange_pcont_container_size_neg prange_pcont_container_size_pos \
	prange_pcont_container_local_size_neg \
	prange_pcont_container_local_size_pos \
	prange_pcont_empty_neg prange_pcont_empty_pos \
	prange_pcont_local_empty_neg prange_pcont_local_empty_pos \
	prange_pcont_clear_pos \
	prange_pcont_local_clear_pos \
	prange_pcont_local_begin_neg prange_pcont_local_begin_pos \
	prange_pcont_local_end_neg prange_pcont_local_end_pos \
	prange_pcont_local_rbegin_neg prange_pcont_local_rbegin_pos \
	prange_pcont_local_rend_neg prange_pcont_local_rend_pos \
	prange_pcont_get_element_neg prange_pcont_get_element_pos \
	prange_pcont_set_element_neg prange_pcont_set_element_pos \
	prange_pcont_update_element_neg prange_pcont_update_element_pos \
	prange_pcont_delete_element_neg prange_pcont_delete_element_pos \
	prange_pcont_add_element_neg prange_pcont_add_element_pos \
	prange_pcont_add_elements_neg prange_pcont_add_elements_pos \
	prange_pcont_is_local_neg prange_pcont_is_local_pos \
	prange_pcont_lookup_neg prange_pcont_lookup_pos \
	prange_pcont_find_remote_pid_neg prange_pcont_find_remote_pid_pos \
	prange_pcont_complete_search_neg prange_pcont_complete_search_pos \
	linear_prange_cnstrctr_default_neg linear_prange_cnstrctr_default_pos \
	linear_prange_cnstrctr_bndry_neg linear_prange_cnstrctr_bndry_pos \
	linear_prange_cnstrctr_cntnr_bndry_neg \
	linear_prange_cnstrctr_cntnr_bndry_pos \
	linear_prange_cnstrctr_cntnr_neg linear_prange_cnstrctr_cntnr_pos \
	linear_prange_cnstrctr_gid_bndry_neg \
	linear_prange_cnstrctr_gid_bndry_pos \
	linear_prange_cnstrctr_gid_cntnr_bndry_neg \
	linear_prange_cnstrctr_gid_cntnr_bndry_pos \
	linear_prange_cnstrctr_copy_neg linear_prange_cnstrctr_copy_pos \
	linear_prange_assignment_oper_neg linear_prange_assignment_oper_pos \
	linear_prange_local_size_neg linear_prange_local_size_pos \
	linear_prange_is_invalid_neg linear_prange_is_invalid_pos \
	linear_prange_get_cont_vers_neg linear_prange_get_cont_vers_pos \
	linear_prange_get_global_id_neg linear_prange_get_global_id_pos \
	linear_prange_get_gid_prod_neg linear_prange_get_gid_prod_pos \
	linear_prange_get_thread_id_neg linear_prange_get_thread_id_pos \
	linear_prange_get_handle_neg linear_prange_get_handle_pos \
	linear_prange_get_boundary_neg linear_prange_get_boundary_pos \
	linear_prange_get_subranges_neg linear_prange_get_subranges_pos \
	linear_prange_get_subrange_neg linear_prange_get_subrange_pos \
	linear_prange_get_num_subranges_neg \
	linear_prange_get_num_subranges_pos \
	linear_prange_set_thread_id_neg linear_prange_set_thread_id_pos \
	linear_prange_set_bndry_neg linear_prange_set_bndry_pos \
	linear_prange_add_subrange_neg linear_prange_add_subrange_pos \
	linear_prange_pcont_container_size_neg \
	linear_prange_pcont_container_size_pos \
	linear_prange_pcont_container_local_size_neg \
	linear_prange_pcont_container_local_size_pos \
	linear_prange_pcont_empty_neg linear_prange_pcont_empty_pos \
	linear_prange_pcont_local_empty_neg \
	linear_prange_pcont_local_empty_pos \
	linear_prange_pcont_clear_pos \
	linear_prange_pcont_local_clear_pos \
	linear_prange_pcont_local_begin_neg \
	linear_prange_pcont_local_begin_pos \
	linear_prange_pcont_local_end_neg \
	linear_prange_pcont_local_end_pos \
	linear_prange_pcont_local_rbegin_neg \
	linear_prange_pcont_local_rbegin_pos \
	linear_prange_pcont_local_rend_neg \
	linear_prange_pcont_local_rend_pos \
	linear_prange_pcont_get_element_neg \
	linear_prange_pcont_get_element_pos \
	linear_prange_pcont_set_element_neg \
	linear_prange_pcont_set_element_pos \
	linear_prange_pcont_update_element_neg \
	linear_prange_pcont_update_element_pos \
	linear_prange_pcont_delete_element_neg \
	linear_prange_pcont_delete_element_pos \
	linear_prange_pcont_add_element_neg \
	linear_prange_pcont_add_element_pos \
	linear_prange_pcont_add_elements_neg \
	linear_prange_pcont_add_elements_pos \
	linear_prange_pcont_is_local_neg linear_prange_pcont_is_local_pos \
	linear_prange_pcont_lookup_neg linear_prange_pcont_lookup_pos \
	linear_prange_pcont_find_remote_pid_neg \
	linear_prange_pcont_find_remote_pid_pos \
	linear_prange_pcont_complete_search_neg \
	linear_prange_pcont_complete_search_pos \
	linear_prange_pcont_max_size_neg linear_prange_pcont_max_size_pos \
	linear_prange_pcont_front_neg linear_prange_pcont_front_pos \
	linear_prange_pcont_back_neg linear_prange_pcont_back_pos \
	linear_prange_pcont_insert_neg linear_prange_pcont_insert_pos \
	linear_prange_pcont_erase_neg linear_prange_pcont_erase_pos \
	linear_prange_pcont_push_back_neg linear_prange_pcont_push_back_pos \
	linear_prange_pcont_pop_back_pos \
	linear_prange_pcont_init_bndry_info_neg \
	linear_prange_pcont_init_bndry_info_pos \
	psum pinner pforall pforeach *.o *~

pinner : innerProduct.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

test_pinner : pinner
	$(call staplrun,1) ./pinner 100
	$(call staplrun,4) ./pinner 100
	
psum : pSum.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

test_psum : psum
	$(call staplrun,1) ./psum 100
	$(call staplrun,4) ./psum 100

pforeach : p_for_each.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

test_pforeach : pforeach
	$(call staplrun,1) ./pforeach 100
	$(call staplrun,4) ./pforeach 100

pforall : simple_pForAll.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

test_pforall : pforall
	$(call staplrun,1) ./pforall
	$(call staplrun,4) ./pforall

temp : template.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

counted_bndry_at_start_neg : counted_bndry_at_start_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_at_start_neg

counted_bndry_at_start_pos : counted_bndry_at_start_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_at_start : counted_bndry_at_start_pos
	perl ./count_errors.pl "counted boundary at_start neg... " counted_bndry_at_start_neg 2
	$(call staplrun,1) counted_bndry_at_start_pos
	$(call staplrun,4) counted_bndry_at_start_pos

counted_bndry_at_finish_neg : counted_bndry_at_finish_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_at_finish_neg

counted_bndry_at_finish_pos : counted_bndry_at_finish_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_at_finish : counted_bndry_at_finish_pos
	perl ./count_errors.pl "counted boundary at_finish neg... " counted_bndry_at_finish_neg 2
	$(call staplrun,1) counted_bndry_at_finish_pos
	$(call staplrun,4) counted_bndry_at_finish_pos

counted_bndry_start_accessor_neg : counted_bndry_start_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_start_accessor_neg

counted_bndry_start_accessor_pos : counted_bndry_start_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_start_accessor : counted_bndry_start_accessor_pos
	perl ./count_errors.pl "counted boundary start accessor neg... " counted_bndry_start_accessor_neg 1
	$(call staplrun,1) counted_bndry_start_accessor_pos
	$(call staplrun,4) counted_bndry_start_accessor_pos

counted_bndry_finish_accessor_neg : counted_bndry_finish_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_finish_accessor_neg

counted_bndry_finish_accessor_pos : counted_bndry_finish_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_finish_accessor : counted_bndry_finish_accessor_pos
	perl ./count_errors.pl "counted boundary finish accessor neg... " counted_bndry_finish_accessor_neg 1
	$(call staplrun,1) counted_bndry_finish_accessor_pos
	$(call staplrun,4) counted_bndry_finish_accessor_pos

counted_bndry_start_modifier_neg : counted_bndry_start_modifier_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_start_modifier_neg

counted_bndry_start_modifier_pos : counted_bndry_start_modifier_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_start_modifier : counted_bndry_start_modifier_pos
	perl ./count_errors.pl "counted boundary start modifier neg... " counted_bndry_start_modifier_neg 1
	$(call staplrun,1) counted_bndry_start_modifier_pos
	$(call staplrun,4) counted_bndry_start_modifier_pos

counted_bndry_finish_modifier_neg : counted_bndry_finish_modifier_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_finish_modifier_neg

counted_bndry_finish_modifier_pos : counted_bndry_finish_modifier_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_finish_modifier : counted_bndry_finish_modifier_pos
	perl ./count_errors.pl "counted boundary finish modifier neg... " counted_bndry_finish_modifier_neg 1
	$(call staplrun,1) counted_bndry_finish_modifier_pos
	$(call staplrun,4) counted_bndry_finish_modifier_pos

counted_bndry_size_neg : counted_bndry_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> counted_bndry_size_neg

counted_bndry_size_pos : counted_bndry_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

counted_bndry_size : counted_bndry_size_pos
	perl ./count_errors.pl "counted boundary size neg... " counted_bndry_size_neg 1
	$(call staplrun,1) counted_bndry_size_pos
	$(call staplrun,4) counted_bndry_size_pos

graph_bndry_at_start_neg : graph_bndry_at_start_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> graph_bndry_at_start_neg

graph_bndry_at_start_pos : graph_bndry_at_start_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

graph_bndry_at_start : graph_bndry_at_start_pos
	perl ./count_errors.pl "graph boundary at_start neg... " graph_bndry_at_start_neg 2
	$(call staplrun,1) graph_bndry_at_start_pos
	$(call staplrun,4) graph_bndry_at_start_pos

graph_bndry_at_finish_neg : graph_bndry_at_finish_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> graph_bndry_at_finish_neg

graph_bndry_at_finish_pos : graph_bndry_at_finish_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

graph_bndry_at_finish : graph_bndry_at_finish_pos
	perl ./count_errors.pl "graph boundary at_finish neg... " graph_bndry_at_finish_neg 2
	$(call staplrun,1) graph_bndry_at_finish_pos
	$(call staplrun,4) graph_bndry_at_finish_pos

graph_bndry_start_accessor_neg : graph_bndry_start_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> graph_bndry_start_accessor_neg

graph_bndry_start_accessor_pos : graph_bndry_start_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

graph_bndry_start_accessor : graph_bndry_start_accessor_pos
	perl ./count_errors.pl "graph boundary start accessor neg... " graph_bndry_start_accessor_neg 1
	$(call staplrun,1) graph_bndry_start_accessor_pos
	$(call staplrun,4) graph_bndry_start_accessor_pos

graph_bndry_finish_accessor_neg : graph_bndry_finish_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> graph_bndry_finish_accessor_neg

graph_bndry_finish_accessor_pos : graph_bndry_finish_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

graph_bndry_finish_accessor : graph_bndry_finish_accessor_pos
	perl ./count_errors.pl "graph boundary finish accessor neg... " graph_bndry_finish_accessor_neg 1
	$(call staplrun,1) graph_bndry_finish_accessor_pos
	$(call staplrun,4) graph_bndry_finish_accessor_pos

graph_bndry_size_neg : graph_bndry_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> graph_bndry_size_neg

graph_bndry_size_pos : graph_bndry_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

graph_bndry_size : graph_bndry_size_pos
	perl ./count_errors.pl "graph boundary size neg... " graph_bndry_size_neg 1
	$(call staplrun,1) graph_bndry_size_pos
	$(call staplrun,4) graph_bndry_size_pos

linear_bndry_at_start_neg : linear_bndry_at_start_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_at_start_neg

linear_bndry_at_start_pos : linear_bndry_at_start_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_at_start : linear_bndry_at_start_pos
	perl ./count_errors.pl "linear boundary at_start neg... " linear_bndry_at_start_neg 2
	$(call staplrun,1) linear_bndry_at_start_pos
	$(call staplrun,4) linear_bndry_at_start_pos

linear_bndry_at_finish_neg : linear_bndry_at_finish_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_at_finish_neg

linear_bndry_at_finish_pos : linear_bndry_at_finish_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_at_finish : linear_bndry_at_finish_pos
	perl ./count_errors.pl "linear boundary at_finish neg... " linear_bndry_at_finish_neg 2
	$(call staplrun,1) linear_bndry_at_finish_pos
	$(call staplrun,4) linear_bndry_at_finish_pos

linear_bndry_start_accessor_neg : linear_bndry_start_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_start_accessor_neg

linear_bndry_start_accessor_pos : linear_bndry_start_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_start_accessor : linear_bndry_start_accessor_pos
	perl ./count_errors.pl "linear boundary start accessor neg... " linear_bndry_start_accessor_neg 1
	$(call staplrun,1) linear_bndry_start_accessor_pos
	$(call staplrun,4) linear_bndry_start_accessor_pos

linear_bndry_finish_accessor_neg : linear_bndry_finish_accessor_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_finish_accessor_neg

linear_bndry_finish_accessor_pos : linear_bndry_finish_accessor_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_finish_accessor : linear_bndry_finish_accessor_pos
	perl ./count_errors.pl "linear boundary finish accessor neg... " linear_bndry_finish_accessor_neg 1
	$(call staplrun,1) linear_bndry_finish_accessor_pos
	$(call staplrun,4) linear_bndry_finish_accessor_pos

linear_bndry_start_modifier_neg : linear_bndry_start_modifier_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_start_modifier_neg

linear_bndry_start_modifier_pos : linear_bndry_start_modifier_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_start_modifier : linear_bndry_start_modifier_pos
	perl ./count_errors.pl "linear boundary start modifier neg... " linear_bndry_start_modifier_neg 1
	$(call staplrun,1) linear_bndry_start_modifier_pos
	$(call staplrun,4) linear_bndry_start_modifier_pos

linear_bndry_finish_modifier_neg : linear_bndry_finish_modifier_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_finish_modifier_neg

linear_bndry_finish_modifier_pos : linear_bndry_finish_modifier_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_finish_modifier : linear_bndry_finish_modifier_pos
	perl ./count_errors.pl "linear boundary finish modifier neg... " linear_bndry_finish_modifier_neg 1
	$(call staplrun,1) linear_bndry_finish_modifier_pos
	$(call staplrun,4) linear_bndry_finish_modifier_pos

linear_bndry_size_neg : linear_bndry_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_bndry_size_neg

linear_bndry_size_pos : linear_bndry_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_bndry_size : linear_bndry_size_pos
	perl ./count_errors.pl "linear boundary size neg... " linear_bndry_size_neg 1
	$(call staplrun,1) linear_bndry_size_pos
	$(call staplrun,4) linear_bndry_size_pos

prange_set_ddg_neg : prange_set_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_set_ddg_neg

prange_set_ddg_pos : prange_set_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_set_ddg : prange_set_ddg_pos
	perl ./count_errors.pl "pRange set_ddg neg... " prange_set_ddg_neg 1
	$(call staplrun,1) prange_set_ddg_pos
	$(call staplrun,4) prange_set_ddg_pos

prange_get_ddg_neg : prange_get_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_ddg_neg

prange_get_ddg_pos : prange_get_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_ddg : prange_get_ddg_pos
	perl ./count_errors.pl "pRange get_ddg neg... " prange_get_ddg_neg 1
	$(call staplrun,1) prange_get_ddg_pos
	$(call staplrun,4) prange_get_ddg_pos

prange_set_elem_ddg_neg : prange_set_elem_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_set_elem_ddg_neg

prange_set_elem_ddg_pos : prange_set_elem_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_set_elem_ddg : prange_set_elem_ddg_pos
	perl ./count_errors.pl "pRange set_elem_ddg neg... " prange_set_elem_ddg_neg 1
	$(call staplrun,1) prange_set_elem_ddg_pos
	$(call staplrun,4) prange_set_elem_ddg_pos

prange_get_elem_ddg_neg : prange_get_elem_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_elem_ddg_neg

prange_get_elem_ddg_pos : prange_get_elem_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_elem_ddg : prange_get_elem_ddg_pos
	perl ./count_errors.pl "pRange get_elem_ddg neg... " prange_get_elem_ddg_neg 2
	$(call staplrun,1) prange_get_elem_ddg_pos
	$(call staplrun,4) prange_get_elem_ddg_pos

linear_prange_set_ddg_neg : linear_prange_set_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_set_ddg_neg

linear_prange_set_ddg_pos : linear_prange_set_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_set_ddg : linear_prange_set_ddg_pos
	perl ./count_errors.pl "Linear pRange set_ddg neg... " linear_prange_set_ddg_neg 1
	$(call staplrun,1) linear_prange_set_ddg_pos
	$(call staplrun,4) linear_prange_set_ddg_pos

linear_prange_get_ddg_neg : linear_prange_get_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_ddg_neg

linear_prange_get_ddg_pos : linear_prange_get_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_ddg : linear_prange_get_ddg_pos
	perl ./count_errors.pl "Linear pRange get_ddg neg... " linear_prange_get_ddg_neg 1
	$(call staplrun,1) linear_prange_get_ddg_pos
	$(call staplrun,4) linear_prange_get_ddg_pos

linear_prange_set_elem_ddg_neg : linear_prange_set_elem_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_set_elem_ddg_neg

linear_prange_set_elem_ddg_pos : linear_prange_set_elem_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_set_elem_ddg : linear_prange_set_elem_ddg_pos
	perl ./count_errors.pl "Linear pRange set_elem_ddg neg... " linear_prange_set_elem_ddg_neg 1
	$(call staplrun,1) linear_prange_set_elem_ddg_pos
	$(call staplrun,4) linear_prange_set_elem_ddg_pos

linear_prange_get_elem_ddg_neg : linear_prange_get_elem_ddg_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_elem_ddg_neg

linear_prange_get_elem_ddg_pos : linear_prange_get_elem_ddg_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_elem_ddg : linear_prange_get_elem_ddg_pos
	perl ./count_errors.pl "Linear pRange get_elem_ddg neg... " linear_prange_get_elem_ddg_neg 2
	$(call staplrun,1) linear_prange_get_elem_ddg_pos
	$(call staplrun,4) linear_prange_get_elem_ddg_pos

pddg_add_task_neg : pddg_add_task_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_add_task_neg

pddg_add_task_pos : pddg_add_task_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

pddg_add_task : pddg_add_task_pos
	perl ./count_errors.pl "pDDG add_task neg... " pddg_add_task_neg 1
	$(call staplrun,1) pddg_add_task_pos
	$(call staplrun,4) pddg_add_task_pos

pddg_remove_task_neg : pddg_remove_task_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_remove_task_neg

pddg_remove_task_pos : pddg_remove_task_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

pddg_remove_task : pddg_remove_task_pos
	perl ./count_errors.pl "pDDG remove_task neg... " pddg_remove_task_neg 1
	$(call staplrun,1) pddg_remove_task_pos
	$(call staplrun,4) pddg_remove_task_pos

pddg_add_dependence_neg : pddg_add_dependence_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_add_dependence_neg

pddg_add_dependence_pos : pddg_add_dependence_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

pddg_add_dependence : pddg_add_dependence_pos
	perl ./count_errors.pl "pDDG add_dependence neg... " pddg_add_dependence_neg 2
	$(call staplrun,1) pddg_add_dependence_pos
	$(call staplrun,4) pddg_add_dependence_pos
	
pddg_remove_dependence_neg : pddg_remove_dependence_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_remove_dependence_neg

pddg_remove_dependence_pos : pddg_remove_dependence_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

pddg_remove_dependence : pddg_remove_dependence_pos
	perl ./count_errors.pl "pDDG remove_dependence neg... " pddg_remove_dependence_neg 2
	$(call staplrun,1) pddg_remove_dependence_pos
	$(call staplrun,4) pddg_remove_dependence_pos

pddg_get_ready_neg : pddg_get_ready_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_get_ready_neg

pddg_get_ready_pos : pddg_get_ready_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

pddg_get_ready : pddg_get_ready_pos
	perl ./count_errors.pl "pDDG get_ready neg... " pddg_get_ready_neg 1
	$(call staplrun,1) pddg_get_ready_pos
	$(call staplrun,4) pddg_get_ready_pos

pddg_processed_neg : pddg_processed_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_processed_neg

pddg_processed_pos : pddg_processed_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

pddg_processed : pddg_processed_pos
	perl ./count_errors.pl "pDDG processed neg... " pddg_processed_neg 1
	$(call staplrun,1) pddg_processed_pos
	$(call staplrun,4) pddg_processed_pos

pddg_finished_neg : pddg_finished_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> pddg_finished_neg

pddg_finished_pos : pddg_finished_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

pddg_finished : pddg_finished_pos
	perl ./count_errors.pl "pDDG finished neg... " pddg_finished_neg 1
	$(call staplrun,1) pddg_finished_pos
	$(call staplrun,4) pddg_finished_pos

pddg_reset_pos : pddg_reset_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

pddg_reset : pddg_reset_pos
	$(call staplrun,1) pddg_reset_pos
	$(call staplrun,4) pddg_reset_pos

ddg_add_task_neg : ddg_add_task_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_add_task_neg

ddg_add_task_pos : ddg_add_task_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

ddg_add_task : ddg_add_task_pos
	perl ./count_errors.pl "DDG add_task neg... " ddg_add_task_neg 1
	$(call staplrun,1) ddg_add_task_pos
	$(call staplrun,4) ddg_add_task_pos

ddg_remove_task_neg : ddg_remove_task_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_remove_task_neg

ddg_remove_task_pos : ddg_remove_task_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

ddg_remove_task : ddg_remove_task_pos
	perl ./count_errors.pl "DDG remove_task neg... " ddg_remove_task_neg 1
	$(call staplrun,1) ddg_remove_task_pos
	$(call staplrun,4) ddg_remove_task_pos

ddg_add_dependence_neg : ddg_add_dependence_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_add_dependence_neg

ddg_add_dependence_pos : ddg_add_dependence_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

ddg_add_dependence : ddg_add_dependence_pos
	perl ./count_errors.pl "DDG add_dependence neg... " ddg_add_dependence_neg 2
	$(call staplrun,1) ddg_add_dependence_pos
	$(call staplrun,4) ddg_add_dependence_pos
	
ddg_remove_dependence_neg : ddg_remove_dependence_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_remove_dependence_neg

ddg_remove_dependence_pos : ddg_remove_dependence_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB}

ddg_remove_dependence : ddg_remove_dependence_pos
	perl ./count_errors.pl "DDG remove_dependence neg... " ddg_remove_dependence_neg 2
	$(call staplrun,1) ddg_remove_dependence_pos
	$(call staplrun,4) ddg_remove_dependence_pos

ddg_get_ready_neg : ddg_get_ready_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_get_ready_neg

ddg_get_ready_pos : ddg_get_ready_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

ddg_get_ready : ddg_get_ready_pos
	perl ./count_errors.pl "DDG get_ready neg... " ddg_get_ready_neg 1
	$(call staplrun,1) ddg_get_ready_pos
	$(call staplrun,4) ddg_get_ready_pos

ddg_processed_neg : ddg_processed_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_processed_neg

ddg_processed_pos : ddg_processed_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

ddg_processed : ddg_processed_pos
	perl ./count_errors.pl "DDG processed neg... " ddg_processed_neg 1
	$(call staplrun,1) ddg_processed_pos
	$(call staplrun,4) ddg_processed_pos

ddg_finished_neg : ddg_finished_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> ddg_finished_neg

ddg_finished_pos : ddg_finished_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

ddg_finished : ddg_finished_pos
	perl ./count_errors.pl "DDG finished neg... " ddg_finished_neg 1
	$(call staplrun,1) ddg_finished_pos
	$(call staplrun,4) ddg_finished_pos

ddg_reset_pos : ddg_reset_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

ddg_reset : ddg_reset_pos
	$(call staplrun,1) ddg_reset_pos
	$(call staplrun,4) ddg_reset_pos

prange_cnstrctr_default_neg : prange_cnstrctr_default_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_default_neg

prange_cnstrctr_default_pos : prange_cnstrctr_default_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_default : prange_cnstrctr_default_pos
	perl ./count_errors.pl "pRange default constructor neg... "  prange_cnstrctr_default_neg 1
	$(call staplrun,1) prange_cnstrctr_default_pos
	$(call staplrun,4) prange_cnstrctr_default_pos

prange_cnstrctr_bndry_neg : prange_cnstrctr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_bndry_neg

prange_cnstrctr_bndry_pos : prange_cnstrctr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_bndry : prange_cnstrctr_bndry_pos
	perl ./count_errors.pl "pRange bndry constructor neg... " prange_cnstrctr_bndry_neg 1
	$(call staplrun,1) prange_cnstrctr_bndry_pos
	$(call staplrun,4) prange_cnstrctr_bndry_pos

prange_cnstrctr_cntnr_bndry_neg : prange_cnstrctr_cntnr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_cntnr_bndry_neg

prange_cnstrctr_cntnr_bndry_pos : prange_cnstrctr_cntnr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_cntnr_bndry : prange_cnstrctr_cntnr_bndry_pos
	perl ./count_errors.pl "pRange cntnr/bndry constructor neg... " prange_cnstrctr_cntnr_bndry_neg 3
	$(call staplrun,1) prange_cnstrctr_cntnr_bndry_pos
	$(call staplrun,4) prange_cnstrctr_cntnr_bndry_pos

prange_cnstrctr_cntnr_neg : prange_cnstrctr_cntnr_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_cntnr_neg

prange_cnstrctr_cntnr_pos : prange_cnstrctr_cntnr_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_cntnr : prange_cnstrctr_cntnr_pos
	perl ./count_errors.pl "pRange cntnr constructor neg... " prange_cnstrctr_cntnr_neg 2
	$(call staplrun,1) prange_cnstrctr_cntnr_pos
	$(call staplrun,4) prange_cnstrctr_cntnr_pos

prange_cnstrctr_gid_bndry_neg : prange_cnstrctr_gid_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_gid_bndry_neg

prange_cnstrctr_gid_bndry_pos : prange_cnstrctr_gid_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_gid_bndry : prange_cnstrctr_gid_bndry_pos
	perl ./count_errors.pl "pRange GID/bndry constructor neg... " prange_cnstrctr_gid_bndry_neg 3
	$(call staplrun,1) prange_cnstrctr_gid_bndry_pos
	$(call staplrun,4) prange_cnstrctr_gid_bndry_pos

prange_cnstrctr_gid_cntnr_bndry_neg : prange_cnstrctr_gid_cntnr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_gid_cntnr_bndry_neg

prange_cnstrctr_gid_cntnr_bndry_pos : prange_cnstrctr_gid_cntnr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_gid_cntnr_bndry : prange_cnstrctr_gid_cntnr_bndry_pos
	perl ./count_errors.pl "pRange GID/cntnr/bndry constructor neg... " prange_cnstrctr_gid_cntnr_bndry_neg 4
	$(call staplrun,1) prange_cnstrctr_gid_cntnr_bndry_pos
	$(call staplrun,4) prange_cnstrctr_gid_cntnr_bndry_pos

prange_cnstrctr_copy_neg : prange_cnstrctr_copy_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_cnstrctr_copy_neg 

prange_cnstrctr_copy_pos : prange_cnstrctr_copy_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_cnstrctr_copy : prange_cnstrctr_copy_pos
	perl ./count_errors.pl "pRange copy constructor neg... " prange_cnstrctr_copy_neg 1
	$(call staplrun,1) prange_cnstrctr_copy_pos
	$(call staplrun,4) prange_cnstrctr_copy_pos

prange_assignment_oper_neg : prange_assignment_oper_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_assignment_oper_neg

prange_assignment_oper_pos : prange_assignment_oper_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_assignment_oper : prange_assignment_oper_pos
	perl ./count_errors.pl "pRange assignment operator neg... " prange_assignment_oper_neg 1
	$(call staplrun,1) prange_assignment_oper_pos
	$(call staplrun,4) prange_assignment_oper_pos

prange_local_size_neg : prange_local_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_local_size_neg 

prange_local_size_pos : prange_local_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_local_size : prange_local_size_pos
	perl ./count_errors.pl "pRange local_size neg... " prange_local_size_neg 1
	$(call staplrun,1) prange_local_size_pos
	$(call staplrun,4) prange_local_size_pos

prange_is_invalid_neg : prange_is_invalid_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_is_invalid_neg 

prange_is_invalid_pos : prange_is_invalid_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_is_invalid : prange_is_invalid_pos
	perl ./count_errors.pl "pRange is_invalid neg... " prange_is_invalid_neg 1
	$(call staplrun,1) prange_is_invalid_pos
	$(call staplrun,4) prange_is_invalid_pos

prange_get_cont_vers_neg : prange_get_cont_vers_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_cont_vers_neg 

prange_get_cont_vers_pos : prange_get_cont_vers_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_cont_vers : prange_get_cont_vers_pos
	perl ./count_errors.pl "pRange get_container_version neg... " prange_get_cont_vers_neg 1
	$(call staplrun,1) prange_get_cont_vers_pos
	$(call staplrun,4) prange_get_cont_vers_pos

prange_get_global_id_neg : prange_get_global_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_global_id_neg 

prange_get_global_id_pos : prange_get_global_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_global_id : prange_get_global_id_pos
	perl ./count_errors.pl "pRange get_global_id neg... " prange_get_global_id_neg 1
	$(call staplrun,1) prange_get_global_id_pos
	$(call staplrun,4) prange_get_global_id_pos

prange_get_gid_prod_neg : prange_get_gid_prod_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_gid_prod_neg 

prange_get_gid_prod_pos : prange_get_gid_prod_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_gid_prod : prange_get_gid_prod_pos
	perl ./count_errors.pl "pRange get_gid_producer neg... " prange_get_gid_prod_neg 1
	$(call staplrun,1) prange_get_gid_prod_pos
	$(call staplrun,4) prange_get_gid_prod_pos

prange_get_thread_id_neg : prange_get_thread_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_thread_id_neg 

prange_get_thread_id_pos : prange_get_thread_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_thread_id : prange_get_thread_id_pos
	perl ./count_errors.pl "pRange get_thread_id neg... " prange_get_thread_id_neg 1
	$(call staplrun,1) prange_get_thread_id_pos
	$(call staplrun,4) prange_get_thread_id_pos

prange_get_handle_neg : prange_get_handle_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_handle_neg 

prange_get_handle_pos : prange_get_handle_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_handle : prange_get_handle_pos
	perl ./count_errors.pl "pRange get_handle neg... " prange_get_handle_neg 1
	$(call staplrun,1) prange_get_handle_pos
	$(call staplrun,4) prange_get_handle_pos

prange_get_boundary_neg : prange_get_boundary_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_boundary_neg 

prange_get_boundary_pos : prange_get_boundary_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_boundary : prange_get_boundary_pos
	perl ./count_errors.pl "pRange get_boundary neg... " prange_get_boundary_neg 1
	$(call staplrun,1) prange_get_boundary_pos
	$(call staplrun,4) prange_get_boundary_pos

prange_get_subranges_neg : prange_get_subranges_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_subranges_neg 

prange_get_subranges_pos : prange_get_subranges_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_subranges : prange_get_subranges_pos
	perl ./count_errors.pl "pRange get_subranges neg... " prange_get_subranges_neg 1
	$(call staplrun,1) prange_get_subranges_pos
	$(call staplrun,4) prange_get_subranges_pos

prange_get_subrange_neg : prange_get_subrange_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_subrange_neg 

prange_get_subrange_pos : prange_get_subrange_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_subrange : prange_get_subrange_pos
	perl ./count_errors.pl "pRange get_subrange neg... " prange_get_subrange_neg 2
	$(call staplrun,1) prange_get_subrange_pos
	$(call staplrun,4) prange_get_subrange_pos

prange_get_num_subranges_neg : prange_get_num_subranges_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_get_num_subranges_neg

prange_get_num_subranges_pos : prange_get_num_subranges_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_get_num_subranges : prange_get_num_subranges_pos
	perl ./count_errors.pl "pRange get_num_subranges neg... " prange_get_num_subranges_neg 1
	$(call staplrun,1) prange_get_num_subranges_pos
	$(call staplrun,4) prange_get_num_subranges_pos

prange_set_thread_id_neg : prange_set_thread_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_set_thread_id_neg 

prange_set_thread_id_pos : prange_set_thread_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_set_thread_id : prange_set_thread_id_pos
	perl ./count_errors.pl "pRange set_thread_id neg... " prange_set_thread_id_neg 1
	$(call staplrun,1) prange_set_thread_id_pos
	$(call staplrun,4) prange_set_thread_id_pos

prange_set_bndry_neg : prange_set_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_set_bndry_neg 

prange_set_bndry_pos : prange_set_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_set_bndry : prange_set_bndry_pos
	perl ./count_errors.pl "pRange set_boundary neg... " prange_set_bndry_neg 1
	$(call staplrun,1) prange_set_bndry_pos
	$(call staplrun,4) prange_set_bndry_pos

prange_add_subrange_neg : prange_add_subrange_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_add_subrange_neg 

prange_add_subrange_pos : prange_add_subrange_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_add_subrange : prange_add_subrange_pos
	perl ./count_errors.pl "pRange add_subrange neg... " prange_add_subrange_neg 1
	$(call staplrun,1) prange_add_subrange_pos
	$(call staplrun,4) prange_add_subrange_pos

prange_pcont_container_size_neg : prange_pcont_container_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_container_size_neg

prange_pcont_container_size_pos : prange_pcont_container_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_container_size : prange_pcont_container_size_pos
	perl ./count_errors.pl "pRange container_size neg... " prange_pcont_container_size_neg 2
	$(call staplrun,1) prange_pcont_container_size_pos
	$(call staplrun,4) prange_pcont_container_size_pos

prange_pcont_container_local_size_neg : prange_pcont_container_local_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_container_local_size_neg

prange_pcont_container_local_size_pos : prange_pcont_container_local_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_container_local_size : prange_pcont_container_local_size_pos
	perl ./count_errors.pl "pRange container_local_size neg... " prange_pcont_container_local_size_neg 2
	$(call staplrun,1) prange_pcont_container_local_size_pos
	$(call staplrun,4) prange_pcont_container_local_size_pos

prange_pcont_empty_neg : prange_pcont_empty_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_empty_neg 

prange_pcont_empty_pos : prange_pcont_empty_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_empty : prange_pcont_empty_pos
	perl ./count_errors.pl "pRange empty neg... " prange_pcont_empty_neg 2
	$(call staplrun,1) prange_pcont_empty_pos
	$(call staplrun,4) prange_pcont_empty_pos

prange_pcont_local_empty_neg : prange_pcont_local_empty_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_local_empty_neg

prange_pcont_local_empty_pos : prange_pcont_local_empty_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_empty : prange_pcont_local_empty_pos
	perl ./count_errors.pl "pRange local_empty neg... " prange_pcont_local_empty_neg 2
	$(call staplrun,1) prange_pcont_local_empty_pos
	$(call staplrun,4) prange_pcont_local_empty_pos

prange_pcont_clear_pos : prange_pcont_clear_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_clear : prange_pcont_clear_pos
	$(call staplrun,1) prange_pcont_clear_pos
	$(call staplrun,4) prange_pcont_clear_pos

prange_pcont_local_clear_pos : prange_pcont_local_clear_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_clear : prange_pcont_local_clear_pos
	$(call staplrun,1) prange_pcont_local_clear_pos
	$(call staplrun,4) prange_pcont_local_clear_pos

prange_pcont_local_begin_neg : prange_pcont_local_begin_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_local_begin_neg

prange_pcont_local_begin_pos : prange_pcont_local_begin_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_begin : prange_pcont_local_begin_pos
	perl ./count_errors.pl "pRange local_begin neg... " prange_pcont_local_begin_neg 2
	$(call staplrun,1) prange_pcont_local_begin_pos
	$(call staplrun,4) prange_pcont_local_begin_pos

prange_pcont_local_end_neg : prange_pcont_local_end_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_local_end_neg

prange_pcont_local_end_pos : prange_pcont_local_end_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_end : prange_pcont_local_end_pos
	perl ./count_errors.pl "pRange local_end neg... " prange_pcont_local_end_neg 2
	$(call staplrun,1) prange_pcont_local_end_pos
	$(call staplrun,4) prange_pcont_local_end_pos

prange_pcont_local_rbegin_neg : prange_pcont_local_rbegin_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_local_rbegin_neg

prange_pcont_local_rbegin_pos : prange_pcont_local_rbegin_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_rbegin : prange_pcont_local_rbegin_pos
	perl ./count_errors.pl "pRange local_rbegin neg... " prange_pcont_local_rbegin_neg 2
	$(call staplrun,1) prange_pcont_local_rbegin_pos
	$(call staplrun,4) prange_pcont_local_rbegin_pos

prange_pcont_local_rend_neg : prange_pcont_local_rend_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_local_rend_neg

prange_pcont_local_rend_pos : prange_pcont_local_rend_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_local_rend : prange_pcont_local_rend_pos
	perl ./count_errors.pl "pRange local_rend neg... " prange_pcont_local_rend_neg 2
	$(call staplrun,1) prange_pcont_local_rend_pos
	$(call staplrun,4) prange_pcont_local_rend_pos

prange_pcont_get_element_neg : prange_pcont_get_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_get_element_neg

prange_pcont_get_element_pos : prange_pcont_get_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_get_element : prange_pcont_get_element_pos
	perl ./count_errors.pl "pRange get_element neg... " prange_pcont_get_element_neg 4
	$(call staplrun,1) prange_pcont_get_element_pos
	$(call staplrun,4) prange_pcont_get_element_pos

prange_pcont_set_element_neg : prange_pcont_set_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_set_element_neg

prange_pcont_set_element_pos : prange_pcont_set_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_set_element : prange_pcont_set_element_pos
	perl ./count_errors.pl "pRange set_element neg... " prange_pcont_set_element_neg 4
	$(call staplrun,1) prange_pcont_set_element_pos
	$(call staplrun,4) prange_pcont_set_element_pos

prange_pcont_update_element_neg : prange_pcont_update_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_update_element_neg

prange_pcont_update_element_pos : prange_pcont_update_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_update_element : prange_pcont_update_element_pos
	perl ./count_errors.pl "pRange update_element neg... " prange_pcont_update_element_neg 6
	$(call staplrun,1) prange_pcont_update_element_pos
	$(call staplrun,4) prange_pcont_update_element_pos

prange_pcont_delete_element_neg : prange_pcont_delete_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_delete_element_neg

prange_pcont_delete_element_pos : prange_pcont_delete_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_delete_element : prange_pcont_delete_element_pos
	perl ./count_errors.pl "pRange delete_element neg... " prange_pcont_delete_element_neg 1
	$(call staplrun,1) prange_pcont_delete_element_pos
	$(call staplrun,4) prange_pcont_delete_element_pos

prange_pcont_add_element_neg : prange_pcont_add_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_add_element_neg

prange_pcont_add_element_pos : prange_pcont_add_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_add_element : prange_pcont_add_element_pos
	perl ./count_errors.pl "pRange add_element neg... " prange_pcont_add_element_neg 1
	$(call staplrun,1) prange_pcont_add_element_pos
	$(call staplrun,4) prange_pcont_add_element_pos

prange_pcont_add_elements_neg : prange_pcont_add_elements_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_add_elements_neg

prange_pcont_add_elements_pos : prange_pcont_add_elements_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_add_elements : prange_pcont_add_elements_pos
	perl ./count_errors.pl "pRange add_elements neg... " prange_pcont_add_elements_neg 1
	$(call staplrun,1) prange_pcont_add_elements_pos
	$(call staplrun,4) prange_pcont_add_elements_pos

prange_pcont_is_local_neg : prange_pcont_is_local_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_is_local_neg

prange_pcont_is_local_pos : prange_pcont_is_local_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_is_local : prange_pcont_is_local_pos
	perl ./count_errors.pl "pRange is_local neg... " prange_pcont_is_local_neg 10
	$(call staplrun,1) prange_pcont_is_local_pos
	$(call staplrun,4) prange_pcont_is_local_pos

prange_pcont_lookup_neg : prange_pcont_lookup_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_lookup_neg 

prange_pcont_lookup_pos : prange_pcont_lookup_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_lookup  : prange_pcont_lookup_pos
	perl ./count_errors.pl "pRange lookup neg... " prange_pcont_lookup_neg 4
	$(call staplrun,1) prange_pcont_lookup_pos
	$(call staplrun,4) prange_pcont_lookup_pos

prange_pcont_find_remote_pid_neg : prange_pcont_find_remote_pid_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_find_remote_pid_neg

prange_pcont_find_remote_pid_pos : prange_pcont_find_remote_pid_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_find_remote_pid : prange_pcont_find_remote_pid_pos
	perl ./count_errors.pl "pRange find_remote_pid neg... " prange_pcont_find_remote_pid_neg 4
	$(call staplrun,1) prange_pcont_find_remote_pid_pos
	$(call staplrun,4) prange_pcont_find_remote_pid_pos

prange_pcont_complete_search_neg : prange_pcont_complete_search_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> prange_pcont_complete_search_neg

prange_pcont_complete_search_pos : prange_pcont_complete_search_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

prange_pcont_complete_search : prange_pcont_complete_search_pos
	perl ./count_errors.pl "pRange complete_search neg... " prange_pcont_complete_search_neg 4
	$(call staplrun,1) prange_pcont_complete_search_pos
	$(call staplrun,4) prange_pcont_complete_search_pos

linear_prange_cnstrctr_default_neg : linear_prange_cnstrctr_default_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_default_neg

linear_prange_cnstrctr_default_pos : linear_prange_cnstrctr_default_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_default : linear_prange_cnstrctr_default_pos
	perl ./count_errors.pl "Linear pRange default constructor neg... " linear_prange_cnstrctr_default_neg 1
	$(call staplrun,1) linear_prange_cnstrctr_default_pos
	$(call staplrun,4) linear_prange_cnstrctr_default_pos

linear_prange_cnstrctr_bndry_neg : linear_prange_cnstrctr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_bndry_neg

linear_prange_cnstrctr_bndry_pos : linear_prange_cnstrctr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_bndry : linear_prange_cnstrctr_bndry_pos
	perl ./count_errors.pl "Linear pRange boundary constructor neg... " linear_prange_cnstrctr_bndry_neg 1
	$(call staplrun,1) linear_prange_cnstrctr_bndry_pos
	$(call staplrun,4) linear_prange_cnstrctr_bndry_pos

linear_prange_cnstrctr_cntnr_bndry_neg : linear_prange_cnstrctr_cntnr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_cntnr_bndry_neg

linear_prange_cnstrctr_cntnr_bndry_pos : linear_prange_cnstrctr_cntnr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_cntnr_bndry : linear_prange_cnstrctr_cntnr_bndry_pos
	perl ./count_errors.pl "Linear pRange container boundary constructor neg... " linear_prange_cnstrctr_cntnr_bndry_neg 3
	$(call staplrun,1) linear_prange_cnstrctr_cntnr_bndry_pos
	$(call staplrun,4) linear_prange_cnstrctr_cntnr_bndry_pos

linear_prange_cnstrctr_cntnr_neg : linear_prange_cnstrctr_cntnr_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_cntnr_neg

linear_prange_cnstrctr_cntnr_pos : linear_prange_cnstrctr_cntnr_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_cntnr : linear_prange_cnstrctr_cntnr_pos
	perl ./count_errors.pl "Linear pRange container constructor neg... " linear_prange_cnstrctr_cntnr_neg 2
	$(call staplrun,1) linear_prange_cnstrctr_cntnr_pos
	$(call staplrun,4) linear_prange_cnstrctr_cntnr_pos

linear_prange_cnstrctr_gid_bndry_neg : linear_prange_cnstrctr_gid_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_gid_bndry_neg

linear_prange_cnstrctr_gid_bndry_pos : linear_prange_cnstrctr_gid_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_gid_bndry : linear_prange_cnstrctr_gid_bndry_pos
	perl ./count_errors.pl "Linear pRange GID boundary constructor neg... " linear_prange_cnstrctr_gid_bndry_neg 3
	$(call staplrun,1) linear_prange_cnstrctr_gid_bndry_pos
	$(call staplrun,4) linear_prange_cnstrctr_gid_bndry_pos

linear_prange_cnstrctr_gid_cntnr_bndry_neg : linear_prange_cnstrctr_gid_cntnr_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_gid_cntnr_bndry_neg

linear_prange_cnstrctr_gid_cntnr_bndry_pos : linear_prange_cnstrctr_gid_cntnr_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_gid_cntnr_bndry : linear_prange_cnstrctr_gid_cntnr_bndry_pos
	perl ./count_errors.pl "Linear pRange GID container boundary constructor neg... " linear_prange_cnstrctr_gid_cntnr_bndry_neg 4
	$(call staplrun,1) linear_prange_cnstrctr_gid_cntnr_bndry_pos
	$(call staplrun,4) linear_prange_cnstrctr_gid_cntnr_bndry_pos

linear_prange_cnstrctr_copy_neg : linear_prange_cnstrctr_copy_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_cnstrctr_copy_neg

linear_prange_cnstrctr_copy_pos : linear_prange_cnstrctr_copy_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_cnstrctr_copy : linear_prange_cnstrctr_copy_pos
	perl ./count_errors.pl "Linear pRange copy constructor neg... " linear_prange_cnstrctr_copy_neg 1
	$(call staplrun,1) linear_prange_cnstrctr_copy_pos
	$(call staplrun,4) linear_prange_cnstrctr_copy_pos

linear_prange_assignment_oper_neg : linear_prange_assignment_oper_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_assignment_oper_neg

linear_prange_assignment_oper_pos : linear_prange_assignment_oper_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_assignment_oper : linear_prange_assignment_oper_pos
	perl ./count_errors.pl "Linear pRange assignment operator neg... " linear_prange_assignment_oper_neg 1
	$(call staplrun,1) linear_prange_assignment_oper_pos
	$(call staplrun,4) linear_prange_assignment_oper_pos

linear_prange_local_size_neg : linear_prange_local_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_local_size_neg

linear_prange_local_size_pos : linear_prange_local_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_local_size : linear_prange_local_size_pos
	perl ./count_errors.pl "Linear pRange local_size neg... " linear_prange_local_size_neg 1
	$(call staplrun,1) linear_prange_local_size_pos
	$(call staplrun,4) linear_prange_local_size_pos

linear_prange_is_invalid_neg : linear_prange_is_invalid_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_is_invalid_neg

linear_prange_is_invalid_pos : linear_prange_is_invalid_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_is_invalid : linear_prange_is_invalid_pos
	perl ./count_errors.pl "Linear pRange is_invalid neg... " linear_prange_is_invalid_neg 1
	$(call staplrun,1) linear_prange_is_invalid_pos
	$(call staplrun,4) linear_prange_is_invalid_pos

linear_prange_get_cont_vers_neg : linear_prange_get_cont_vers_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_cont_vers_neg

linear_prange_get_cont_vers_pos : linear_prange_get_cont_vers_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_cont_vers : linear_prange_get_cont_vers_pos
	perl ./count_errors.pl "Linear pRange get_container_version neg... " linear_prange_get_cont_vers_neg 1
	$(call staplrun,1) linear_prange_get_cont_vers_pos
	$(call staplrun,4) linear_prange_get_cont_vers_pos

linear_prange_get_global_id_neg : linear_prange_get_global_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_global_id_neg

linear_prange_get_global_id_pos : linear_prange_get_global_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_global_id : linear_prange_get_global_id_pos
	perl ./count_errors.pl "Linear pRange get_global_id neg... " linear_prange_get_global_id_neg 1
	$(call staplrun,1) linear_prange_get_global_id_pos
	$(call staplrun,4) linear_prange_get_global_id_pos

linear_prange_get_gid_prod_neg : linear_prange_get_gid_prod_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_gid_prod_neg

linear_prange_get_gid_prod_pos : linear_prange_get_gid_prod_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_gid_prod : linear_prange_get_gid_prod_pos
	perl ./count_errors.pl "Linear pRange get_gid_producer neg... " linear_prange_get_gid_prod_neg 1
	$(call staplrun,1) linear_prange_get_gid_prod_pos
	$(call staplrun,4) linear_prange_get_gid_prod_pos

linear_prange_get_thread_id_neg : linear_prange_get_thread_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_thread_id_neg

linear_prange_get_thread_id_pos : linear_prange_get_thread_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_thread_id : linear_prange_get_thread_id_pos
	perl ./count_errors.pl "Linear pRange get_thread_id neg... " linear_prange_get_thread_id_neg 1
	$(call staplrun,1) linear_prange_get_thread_id_pos
	$(call staplrun,4) linear_prange_get_thread_id_pos

linear_prange_get_handle_neg : linear_prange_get_handle_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_handle_neg

linear_prange_get_handle_pos : linear_prange_get_handle_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_handle : linear_prange_get_handle_pos
	perl ./count_errors.pl "Linear pRange get_handle neg... " linear_prange_get_handle_neg 1
	$(call staplrun,1) linear_prange_get_handle_pos
	$(call staplrun,4) linear_prange_get_handle_pos

linear_prange_get_boundary_neg : linear_prange_get_boundary_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_boundary_neg

linear_prange_get_boundary_pos : linear_prange_get_boundary_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_boundary : linear_prange_get_boundary_pos
	perl ./count_errors.pl "Linear pRange get_boundary neg... " linear_prange_get_boundary_neg 1
	$(call staplrun,1) linear_prange_get_boundary_pos
	$(call staplrun,4) linear_prange_get_boundary_pos

linear_prange_get_subranges_neg : linear_prange_get_subranges_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_subranges_neg

linear_prange_get_subranges_pos : linear_prange_get_subranges_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_subranges : linear_prange_get_subranges_pos
	perl ./count_errors.pl "Linear pRange get_subranges neg... " linear_prange_get_subranges_neg 1
	$(call staplrun,1) linear_prange_get_subranges_pos
	$(call staplrun,4) linear_prange_get_subranges_pos

linear_prange_get_subrange_neg : linear_prange_get_subrange_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_subrange_neg

linear_prange_get_subrange_pos : linear_prange_get_subrange_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_subrange : linear_prange_get_subrange_pos
	perl ./count_errors.pl "Linear pRange get_subrange neg... " linear_prange_get_subrange_neg 2
	$(call staplrun,1) linear_prange_get_subrange_pos
	$(call staplrun,4) linear_prange_get_subrange_pos

linear_prange_get_num_subranges_neg : linear_prange_get_num_subranges_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_get_num_subranges_neg 

linear_prange_get_num_subranges_pos : linear_prange_get_num_subranges_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_get_num_subranges : linear_prange_get_num_subranges_pos
	perl ./count_errors.pl "Linear pRange get_num_subranges neg... " linear_prange_get_num_subranges_neg 1
	$(call staplrun,1) linear_prange_get_num_subranges_pos
	$(call staplrun,4) linear_prange_get_num_subranges_pos

linear_prange_set_thread_id_neg : linear_prange_set_thread_id_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_set_thread_id_neg 

linear_prange_set_thread_id_pos : linear_prange_set_thread_id_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_set_thread_id : linear_prange_set_thread_id_pos
	perl ./count_errors.pl "Linear pRange set_thread_id neg... " linear_prange_set_thread_id_neg 1
	$(call staplrun,1) linear_prange_set_thread_id_pos
	$(call staplrun,4) linear_prange_set_thread_id_pos


linear_prange_set_bndry_neg : linear_prange_set_bndry_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_set_bndry_neg

linear_prange_set_bndry_pos : linear_prange_set_bndry_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_set_bndry : linear_prange_set_bndry_pos
	perl ./count_errors.pl "Linear pRange set_boundary neg... " linear_prange_set_bndry_neg 1
	$(call staplrun,1) linear_prange_set_bndry_pos
	$(call staplrun,4) linear_prange_set_bndry_pos

linear_prange_add_subrange_neg : linear_prange_add_subrange_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_add_subrange_neg

linear_prange_add_subrange_pos : linear_prange_add_subrange_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_add_subrange : linear_prange_add_subrange_pos
	perl ./count_errors.pl "Linear pRange add_subrange neg... " linear_prange_add_subrange_neg 1
	$(call staplrun,1) linear_prange_add_subrange_pos
	$(call staplrun,4) linear_prange_add_subrange_pos

linear_prange_pcont_container_size_neg : linear_prange_pcont_container_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_container_size_neg

linear_prange_pcont_container_size_pos : linear_prange_pcont_container_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_container_size : linear_prange_pcont_container_size_pos
	perl ./count_errors.pl "Linear pRange container_size neg... " linear_prange_pcont_container_size_neg 2
	$(call staplrun,1) linear_prange_pcont_container_size_pos
	$(call staplrun,4) linear_prange_pcont_container_size_pos

linear_prange_pcont_container_local_size_neg : linear_prange_pcont_container_local_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_container_local_size_neg

linear_prange_pcont_container_local_size_pos : linear_prange_pcont_container_local_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_container_local_size : linear_prange_pcont_container_local_size_pos
	perl ./count_errors.pl "Linear pRange container_local_size neg... " linear_prange_pcont_container_local_size_neg 2
	$(call staplrun,1) linear_prange_pcont_container_local_size_pos
	$(call staplrun,4) linear_prange_pcont_container_local_size_pos

linear_prange_pcont_empty_neg : linear_prange_pcont_empty_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_empty_neg

linear_prange_pcont_empty_pos : linear_prange_pcont_empty_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_empty : linear_prange_pcont_empty_pos
	perl ./count_errors.pl "Linear pRange empty neg... " linear_prange_pcont_empty_neg 2
	$(call staplrun,1) linear_prange_pcont_empty_pos
	$(call staplrun,4) linear_prange_pcont_empty_pos

linear_prange_pcont_local_empty_neg : linear_prange_pcont_local_empty_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_local_empty_neg

linear_prange_pcont_local_empty_pos : linear_prange_pcont_local_empty_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_empty : linear_prange_pcont_local_empty_pos
	perl ./count_errors.pl "Linear pRange local_empty neg... " linear_prange_pcont_local_empty_neg 2
	$(call staplrun,1) linear_prange_pcont_local_empty_pos
	$(call staplrun,4) linear_prange_pcont_local_empty_pos

linear_prange_pcont_clear_pos : linear_prange_pcont_clear_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_clear : linear_prange_pcont_clear_pos
	$(call staplrun,1) linear_prange_pcont_clear_pos
	$(call staplrun,4) linear_prange_pcont_clear_pos

linear_prange_pcont_local_clear_pos : linear_prange_pcont_local_clear_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_clear : linear_prange_pcont_local_clear_pos
	$(call staplrun,1) linear_prange_pcont_local_clear_pos
	$(call staplrun,4) linear_prange_pcont_local_clear_pos

linear_prange_pcont_local_begin_neg : linear_prange_pcont_local_begin_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_local_begin_neg

linear_prange_pcont_local_begin_pos : linear_prange_pcont_local_begin_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_begin : linear_prange_pcont_local_begin_pos
	perl ./count_errors.pl "Linear pRange local_begin neg... " linear_prange_pcont_local_begin_neg 2
	$(call staplrun,1) linear_prange_pcont_local_begin_pos
	$(call staplrun,4) linear_prange_pcont_local_begin_pos

linear_prange_pcont_local_end_neg : linear_prange_pcont_local_end_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_local_end_neg

linear_prange_pcont_local_end_pos : linear_prange_pcont_local_end_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_end : linear_prange_pcont_local_end_pos
	perl ./count_errors.pl "Linear pRange local_end neg... " linear_prange_pcont_local_end_neg 2
	$(call staplrun,1) linear_prange_pcont_local_end_pos
	$(call staplrun,4) linear_prange_pcont_local_end_pos

linear_prange_pcont_local_rbegin_neg : linear_prange_pcont_local_rbegin_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_local_rbegin_neg

linear_prange_pcont_local_rbegin_pos : linear_prange_pcont_local_rbegin_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_rbegin : linear_prange_pcont_local_rbegin_pos
	perl ./count_errors.pl "Linear pRange local_rbegin neg... " linear_prange_pcont_local_rbegin_neg 2
	$(call staplrun,1) linear_prange_pcont_local_rbegin_pos
	$(call staplrun,4) linear_prange_pcont_local_rbegin_pos

linear_prange_pcont_local_rend_neg : linear_prange_pcont_local_rend_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_local_rend_neg

linear_prange_pcont_local_rend_pos : linear_prange_pcont_local_rend_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_local_rend : linear_prange_pcont_local_rend_pos
	perl ./count_errors.pl "Linear pRange local_rend neg... " linear_prange_pcont_local_rend_neg 2
	$(call staplrun,1) linear_prange_pcont_local_rend_pos
	$(call staplrun,4) linear_prange_pcont_local_rend_pos

linear_prange_pcont_get_element_neg : linear_prange_pcont_get_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_get_element_neg

linear_prange_pcont_get_element_pos : linear_prange_pcont_get_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_get_element : linear_prange_pcont_get_element_pos
	perl ./count_errors.pl "Linear pRange get_element neg... " linear_prange_pcont_get_element_neg 4
	$(call staplrun,1) linear_prange_pcont_get_element_pos
	$(call staplrun,4) linear_prange_pcont_get_element_pos

linear_prange_pcont_set_element_neg : linear_prange_pcont_set_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_set_element_neg

linear_prange_pcont_set_element_pos : linear_prange_pcont_set_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_set_element : linear_prange_pcont_set_element_pos
	perl ./count_errors.pl "Linear pRange set_element neg... " linear_prange_pcont_set_element_neg 4
	$(call staplrun,1) linear_prange_pcont_set_element_pos
	$(call staplrun,4) linear_prange_pcont_set_element_pos

linear_prange_pcont_update_element_neg : linear_prange_pcont_update_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_update_element_neg 

linear_prange_pcont_update_element_pos : linear_prange_pcont_update_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_update_element : linear_prange_pcont_update_element_pos
	perl ./count_errors.pl "Linear pRange update_element neg... " linear_prange_pcont_update_element_neg 6
	$(call staplrun,1) linear_prange_pcont_update_element_pos
	$(call staplrun,4) linear_prange_pcont_update_element_pos

linear_prange_pcont_delete_element_neg : linear_prange_pcont_delete_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_delete_element_neg 

linear_prange_pcont_delete_element_pos : linear_prange_pcont_delete_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_delete_element : linear_prange_pcont_delete_element_pos
	perl ./count_errors.pl "Linear pRange delete element neg... " linear_prange_pcont_delete_element_neg 1
	$(call staplrun,1) linear_prange_pcont_delete_element_pos
	$(call staplrun,4) linear_prange_pcont_delete_element_pos

linear_prange_pcont_add_element_neg : linear_prange_pcont_add_element_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_add_element_neg 

linear_prange_pcont_add_element_pos : linear_prange_pcont_add_element_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_add_element : linear_prange_pcont_add_element_pos
	perl ./count_errors.pl "Linear pRange add_element neg... " linear_prange_pcont_add_element_neg 1
	$(call staplrun,1) linear_prange_pcont_add_element_pos
	$(call staplrun,4) linear_prange_pcont_add_element_pos

linear_prange_pcont_add_elements_neg : linear_prange_pcont_add_elements_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_add_elements_neg 

linear_prange_pcont_add_elements_pos : linear_prange_pcont_add_elements_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_add_elements : linear_prange_pcont_add_elements_pos
	perl ./count_errors.pl "Linear pRange add_elements neg... " linear_prange_pcont_add_elements_neg 1
	$(call staplrun,1) linear_prange_pcont_add_elements_pos
	$(call staplrun,4) linear_prange_pcont_add_elements_pos

linear_prange_pcont_is_local_neg : linear_prange_pcont_is_local_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_is_local_neg 

linear_prange_pcont_is_local_pos : linear_prange_pcont_is_local_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_is_local : linear_prange_pcont_is_local_pos
	perl ./count_errors.pl "Linear pRange is_local neg... " linear_prange_pcont_is_local_neg 10
	$(call staplrun,1) linear_prange_pcont_is_local_pos
	$(call staplrun,4) linear_prange_pcont_is_local_pos

linear_prange_pcont_lookup_neg : linear_prange_pcont_lookup_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_lookup_neg 

linear_prange_pcont_lookup_pos : linear_prange_pcont_lookup_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_lookup : linear_prange_pcont_lookup_pos
	perl ./count_errors.pl "Linear pRange lookup neg... " linear_prange_pcont_lookup_neg 4
	$(call staplrun,1) linear_prange_pcont_lookup_pos
	$(call staplrun,4) linear_prange_pcont_lookup_pos

linear_prange_pcont_find_remote_pid_neg : linear_prange_pcont_find_remote_pid_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_find_remote_pid_neg 

linear_prange_pcont_find_remote_pid_pos : linear_prange_pcont_find_remote_pid_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_find_remote_pid : linear_prange_pcont_find_remote_pid_pos
	perl ./count_errors.pl "Linear pRange find_remote_pid neg... " linear_prange_pcont_find_remote_pid_neg 4
	$(call staplrun,1) linear_prange_pcont_find_remote_pid_pos
	$(call staplrun,4) linear_prange_pcont_find_remote_pid_pos

linear_prange_pcont_complete_search_neg : linear_prange_pcont_complete_search_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_complete_search_neg 

linear_prange_pcont_complete_search_pos : linear_prange_pcont_complete_search_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_complete_search : linear_prange_pcont_complete_search_pos
	perl ./count_errors.pl "Linear pRange complete_search neg... " linear_prange_pcont_complete_search_neg 4
	$(call staplrun,1) linear_prange_pcont_complete_search_pos
	$(call staplrun,4) linear_prange_pcont_complete_search_pos

linear_prange_pcont_max_size_neg : linear_prange_pcont_max_size_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_max_size_neg 

linear_prange_pcont_max_size_pos : linear_prange_pcont_max_size_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_max_size : linear_prange_pcont_max_size_pos
	perl ./count_errors.pl "Linear pRange max_size neg... " linear_prange_pcont_max_size_neg 2
	$(call staplrun,1) linear_prange_pcont_max_size_pos
	$(call staplrun,4) linear_prange_pcont_max_size_pos

linear_prange_pcont_front_neg : linear_prange_pcont_front_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_front_neg 

linear_prange_pcont_front_pos : linear_prange_pcont_front_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_front : linear_prange_pcont_front_pos
	perl ./count_errors.pl "Linear pRange front neg... " linear_prange_pcont_front_neg 1
	$(call staplrun,1) linear_prange_pcont_front_pos
	$(call staplrun,4) linear_prange_pcont_front_pos

linear_prange_pcont_back_neg : linear_prange_pcont_back_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_back_neg 

linear_prange_pcont_back_pos : linear_prange_pcont_back_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_back : linear_prange_pcont_back_pos
	perl ./count_errors.pl "Linear pRange back neg... " linear_prange_pcont_back_neg 1
	$(call staplrun,1) linear_prange_pcont_back_pos
	$(call staplrun,4) linear_prange_pcont_back_pos

linear_prange_pcont_insert_neg : linear_prange_pcont_insert_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_insert_neg 

linear_prange_pcont_insert_pos : linear_prange_pcont_insert_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_insert : linear_prange_pcont_insert_pos
	perl ./count_errors.pl "Linear pRange insert neg... " linear_prange_pcont_insert_neg 1
	$(call staplrun,1) linear_prange_pcont_insert_pos
	$(call staplrun,4) linear_prange_pcont_insert_pos

linear_prange_pcont_erase_neg : linear_prange_pcont_erase_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_erase_neg 

linear_prange_pcont_erase_pos : linear_prange_pcont_erase_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_erase : linear_prange_pcont_erase_pos
	perl ./count_errors.pl "Linear pRange erase neg... " linear_prange_pcont_erase_neg 1
	$(call staplrun,1) linear_prange_pcont_erase_pos
	$(call staplrun,4)  linear_prange_pcont_erasepos

linear_prange_pcont_push_back_neg : linear_prange_pcont_push_back_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_push_back_neg 

linear_prange_pcont_push_back_pos : linear_prange_pcont_push_back_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_push_back : linear_prange_pcont_push_back_pos
	perl ./count_errors.pl "Linear pRange push_back neg... " linear_prange_pcont_push_back_neg 1
	$(call staplrun,1) linear_prange_pcont_push_back_pos
	$(call staplrun,4) linear_prange_pcont_push_back_pos

linear_prange_pcont_pop_back_pos : linear_prange_pcont_pop_back_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_pop_back : linear_prange_pcont_pop_back_pos
	$(call staplrun,1) linear_prange_pcont_pop_back_pos
	$(call staplrun,4) linear_prange_pcont_pop_back_pos

linear_prange_pcont_init_bndry_info_neg : linear_prange_pcont_init_bndry_info_neg.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 2> linear_prange_pcont_init_bndry_info_neg 

linear_prange_pcont_init_bndry_info_pos : linear_prange_pcont_init_bndry_info_pos.cc
	${CC} ${CFLAGS} $(MOREFLAGS) -o $@ $< ${LIB} 

linear_prange_pcont_init_bndry_info : linear_prange_pcont_init_bndry_info_pos
	perl ./count_errors.pl "Linear pRange init_boundary_info neg... " linear_prange_pcont_init_bndry_info_neg 1
	$(call staplrun,1) linear_prange_pcont_init_bndry_info_pos
	$(call staplrun,4) linear_prange_pcont_init_bndry_info_pos

#EOF
