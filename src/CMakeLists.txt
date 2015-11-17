include_directories(.)

set(PROX_SRCS
  prox/prox.h
  prox/proxdef.h
  prox/knapsack_eq.h
  prox/knapsack_le.h
  prox/knapsack_le_biased.h
  prox/topk_cone.h
  prox/topk_cone_biased.h
  prox/topk_simplex.h
  prox/topk_simplex_biased.h
  )

set(SOLVE_SRCS
  solve/solve.h
  solve/solvedef.h
  solve/solver.h
  solve/dual_solver.h
  solve/primal_solver.h
  solve/objective_base.h
  solve/l2_hinge_topk.h
  solve/l2_topk_hinge.h
  )

set(UTIL_SRCS
  util/util.h
  util/blas.h
  util/fmath.h
  util/logging.h
  util/numeric.h
  util/stopwatch.h
  )

if(${USE_THREADING_SUFFIX})
  if(${USE_SEQUENTIAL})
    set(MEX_PROX libsdca_prox_st)
    set(MEX_SOLVE libsdca_solve_st)
  else()
    set(MEX_PROX libsdca_prox_mt)
    set(MEX_SOLVE libsdca_solve_mt)
  endif()
else()
  set(MEX_PROX libsdca_prox)
  set(MEX_SOLVE libsdca_solve)
endif()

# Matlab interface
if(${Matlab_FOUND})
  set(MATLAB_API_INSTALL_DIR matlab)
  include_directories(matlab)
  include_directories(${Matlab_INCLUDE_DIRS})

  if(IS_DIRECTORY "${Matlab_SYS_DIR}")
    # A hack to ensure that we link to the libstdc++ shipped with Matlab
    set(CMAKE_SHARED_LINKER_FLAGS "-L${Matlab_SYS_DIR}")
  endif()

  add_definitions(
    -DMEX_PROX="${MEX_PROX}"
    -DMEX_SOLVE="${MEX_SOLVE}"
    )

  matlab_add_mex(
    NAME ${MEX_PROX}
    SRC matlab/mex_prox.cpp matlab/mex_util.h ${PROX_SRCS} ${UTIL_SRCS}
    )

  matlab_add_mex(
    NAME ${MEX_SOLVE}
    SRC matlab/mex_solve.cpp matlab/mex_util.h
        ${SOLVE_SRCS} ${PROX_SRCS} ${UTIL_SRCS}
    LINK_TO ${BLAS_LIBRARIES}
    )

  if(USE_INTEL_OMP_LIBRARY)
    set(_install_dest "${CMAKE_INSTALL_PREFIX}/${MATLAB_API_INSTALL_DIR}")
    add_custom_command(
      TARGET ${MEX_SOLVE}
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${INTEL_OMP_LIBRARY}"
        "${_install_dest}"
      COMMENT "Copying ${INTEL_OMP_LIBRARY} to ${_install_dest}"
    )
    unset(_install_dest)
   endif()

  install(
    TARGETS ${MEX_PROX} ${MEX_SOLVE}
    DESTINATION ${MATLAB_API_INSTALL_DIR}
    )
else()
  message(STATUS "Matlab interface is not compiled.")
endif()