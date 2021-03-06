#ifndef SDCA_SOLVE_L2_HINGE_TOPK_LOSS_H
#define SDCA_SOLVE_L2_HINGE_TOPK_LOSS_H

#include "objective_base.h"
#include "prox/topk_simplex.h"
#include "prox/topk_simplex_biased.h"
#include "solvedef.h"

namespace sdca {

template <typename Data,
          typename Result,
          typename Summation>
struct l2_hinge_topk : public objective_base<Data, Result, Summation> {
  typedef objective_base<Data, Result, Summation> base;
  const difference_type k;
  const Result c;

  l2_hinge_topk(
      const size_type __k,
      const Result __c,
      const Summation __sum
    ) :
      base::objective_base(__c / static_cast<Result>(__k), __sum),
      k(static_cast<difference_type>(__k)),
      c(__c)
  {}

  inline std::string to_string() const {
    std::ostringstream str;
    str << "l2_hinge_topk (k = " << k << ", C = " << c << ")";
    return str.str();
  }

  void update_variables(
      const blas_int num_classes,
      const Data norm2,
      Data* variables,
      Data* scores
      ) const {
    Data *first(variables + 1), *last(variables + num_classes), a(1 / norm2);
    Result rhs(c), rho(1);

    // 1. Prepare a vector to project in 'variables'.
    sdca_blas_axpby(num_classes, a, scores, -1, variables);
    a -= variables[0];
    std::for_each(first, last, [=](Data &x){ x += a; });

    // 2. Proximal step (project 'variables', use 'scores' as scratch space)
    prox_topk_simplex_biased(first, last,
      scores + 1, scores + num_classes, k, rhs, rho, this->sum);

    // 3. Recover the updated variables
    *variables = static_cast<Data>(std::min(rhs,
      this->sum(first, last, static_cast<Result>(0)) ));
    std::for_each(first, last, [](Data &x){ x = -x; });
  }

  inline Result
  primal_loss(
      const blas_int num_classes,
      Data* scores
    ) const {
    Data *first(scores + 1), *last(scores + num_classes), a(1 - scores[0]);
    std::for_each(first, last, [=](Data &x){ x += a; });

    // Find k largest elements
    std::nth_element(first, first + k - 1, last, std::greater<Data>());

    // max{0, sum_k_largest} (division by k happens later)
    return std::max(static_cast<Result>(0),
      this->sum(first, first + k, static_cast<Result>(0)));
  }
};

}

#endif
