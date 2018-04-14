/// This is UFC (Unified Form-assembly Code)
/// This code is released into the public domain.
///
/// The FEniCS Project (http://www.fenicsproject.org/) 2006-2018.
///
/// UFC defines the interface between code generated by FFC
/// and the DOLFIN C++ library. Changes here must be reflected
/// both in the FFC code generation and in the DOLFIN library calls.

#ifndef __UFC_H
#define __UFC_H

#define UFC_VERSION_MAJOR 2018
#define UFC_VERSION_MINOR 1
#define UFC_VERSION_MAINTENANCE 0
#define UFC_VERSION_RELEASE 0

#include <stdint.h>
#include <ufc_geometry.h>

#define CONCAT(a, b, c) #a "." #b "." #c
#define EVALUATOR(a, b, c) CONCAT(a, b, c)

#if UFC_VERSION_RELEASE
const char UFC_VERSION[]
    = EVALUATOR(UFC_VERSION_MAJOR, UFC_VERSION_MINOR, UFC_VERSION_MAINTENANCE);
#else
const char UFC_VERSION[] = EVALUATOR(UFC_VERSION_MAJOR, UFC_VERSION_MINOR,
                                     UFC_VERSION_MAINTENANCE) ".dev0";
#endif

#undef CONCAT
#undef EVALUATOR

enum ufc_shape
{
  interval,
  triangle,
  quadrilateral,
  tetrahedron,
  hexahedron,
  vertex,
  none
};

/// Forward declaration
struct ufc_coordinate_mapping;

typedef struct ufc_finite_element
{
  /// String identifying the finite element
  const char* signature = NULL;

  /// Return the cell shape
  ufc_shape cell_shape = none;

  /// Return the topological dimension of the cell shape
  int topological_dimension = -1;

  /// Return the geometric dimension of the cell shape
  int geometric_dimension = -1;

  /// Return the dimension of the finite element function space
  int space_dimension = -1;

  /// Return the rank of the value space
  int value_rank = -1;

  /// Return the dimension of the value space for axis i
  int (*value_dimension)(int64_t i) = NULL;

  /// Return the number of components of the value space
  int value_size = -1;

  /// Return the rank of the reference value space
  int reference_value_rank = -1;

  /// Return the dimension of the reference value space for axis i
  int (*reference_value_dimension)(int64_t i) = NULL;

  /// Return the number of components of the reference value space
  int reference_value_size = -1;

  /// Return the maximum polynomial degree of the finite element
  /// function space
  int degree = -1;

  /// Return the family of the finite element function space
  const char* family = NULL;

  int (*evaluate_reference_basis)(double* reference_values, int64_t num_points,
                                  const double* X)
      = NULL;

  int (*evaluate_reference_basis_derivatives)(double* reference_values,
                                              int64_t order, int64_t num_points,
                                              const double* X)
      = NULL;

  int (*transform_reference_basis_derivatives)(
      double* values, int64_t order, int64_t num_points,
      const double* reference_values, const double* X, const double* J,
      const double* detJ, const double* K, int cell_orientation)
      = NULL;

  /// Map dofs from vals to values
  void (*map_dofs)(double* values, const double* vals,
                   const double* coordinate_dofs, int cell_orientation,
                   const ufc_coordinate_mapping* cm)
      = NULL;

  // FIXME: change to 'const double* reference_dof_coordinates()'
  /// Tabulate the coordinates of all dofs on a reference cell
  void (*tabulate_reference_dof_coordinates)(double* reference_dof_coordinates)
      = NULL;

  /// Return the number of sub elements (for a mixed element)
  int num_sub_elements = -1;

  /// Create a new finite element for sub element i (for a mixed element)
  ufc_finite_element* (*create_sub_element)(int64_t i) = NULL;

  /// Create a new class instance
  ufc_finite_element* (*create)() = NULL;
} ufc_finite_element;

typedef struct ufc_dofmap
{

  /// Return a string identifying the dofmap
  const char* signature = NULL;

  /// Return the dimension of the local finite element function space
  /// Return the number of dofs with global support (i.e. global constants)
  int64_t num_global_support_dofs = -1;

  /// Return the dimension of the local finite element function space
  /// for a cell (not including global support dofs)
  int64_t num_element_support_dofs = -1;

  /// Return the dimension of the local finite element function space
  /// for a cell (old version including global support dofs)
  int64_t num_element_dofs = -1;

  /// Return the number of dofs on each cell facet
  int64_t num_facet_dofs = -1;

  /// Return the number of dofs associated with each cell entity of
  /// dimension d
  int64_t (*num_entity_dofs)(int64_t d) = NULL;

  /// Return the number of dofs associated with the closure
  /// of each cell entity dimension d
  int64_t (*num_entity_closure_dofs)(int64_t d) = NULL;

  /// Tabulate the local-to-global mapping of dofs on a cell
  ///   num_global_entities[num_entities_per_cell]
  ///   entity_indices[tdim][local_index ]
  void (*tabulate_dofs)(int64_t* dofs, const int64_t* num_global_entities,
                        const int64_t** entity_indices)
      = NULL;

  /// Tabulate the local-to-local mapping from facet dofs to cell dofs
  void (*tabulate_facet_dofs)(int64_t* dofs, int64_t facet) = NULL;

  /// Tabulate the local-to-local mapping of dofs on entity (d, i)
  void (*tabulate_entity_dofs)(int64_t* dofs, int64_t d, int64_t i) = NULL;

  /// Tabulate the local-to-local mapping of dofs on the closure of
  /// entity (d, i)
  void (*tabulate_entity_closure_dofs)(int64_t* dofs, int64_t d, int64_t i)
      = NULL;

  /// Return the number of sub dofmaps (for a mixed element)
  int64_t num_sub_dofmaps = -1;

  /// Create a new dofmap for sub dofmap i (for a mixed element)
  ufc_dofmap* (*create_sub_dofmap)(int64_t i) = NULL;

  /// Create a new class instance
  ufc_dofmap* (*create)() = NULL;
} ufc_dofmap;

/// A representation of a coordinate mapping parameterized by a local
/// finite element basis on each cell
typedef struct ufc_coordinate_mapping
{

  /// Return coordinate_mapping signature string
  const char* signature = NULL;

  /// Create object of the same type
  ufc_coordinate_mapping* (*create)() = NULL;

  /// Return geometric dimension of the coordinate_mapping
  int64_t geometric_dimension = -1;

  /// Return topological dimension of the coordinate_mapping
  int64_t topological_dimension = -1;

  /// Return cell shape of the coordinate_mapping
  ufc_shape cell_shape = none;

  /// Create finite_element object representing the coordinate parameterization
  ufc_finite_element* (*create_coordinate_finite_element)() = NULL;

  /// Create dofmap object representing the coordinate parameterization
  ufc_dofmap* (*create_coordinate_dofmap)() = NULL;

  /// Compute physical coordinates x from reference coordinates X,
  /// the inverse of compute_reference_coordinates
  ///
  /// @param[out] x
  ///         Physical coordinates.
  ///         Dimensions: x[num_points][gdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] X
  ///         Reference cell coordinates.
  ///         Dimensions: X[num_points][tdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  ///
  void (*compute_physical_coordinates)(double* x, int64_t num_points,
                                       const double* X,
                                       const double* coordinate_dofs)
      = NULL;

  /// Compute reference coordinates X from physical coordinates x,
  /// the inverse of compute_physical_coordinates
  ///
  /// @param[out] X
  ///         Reference cell coordinates.
  ///         Dimensions: X[num_points][tdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] x
  ///         Physical coordinates.
  ///         Dimensions: x[num_points][gdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  /// @param[in] cell_orientation
  ///         Orientation of the cell, 1 means flipped w.r.t. reference cell.
  ///         Only relevant on manifolds (tdim < gdim).
  ///
  void (*compute_reference_coordinates)(double* X, int64_t num_points,
                                        const double* x,
                                        const double* coordinate_dofs,
                                        int cell_orientation)
      = NULL;

  /// Compute X, J, detJ, K from physical coordinates x on a cell
  ///
  /// @param[out] X
  ///         Reference cell coordinates.
  ///         Dimensions: X[num_points][tdim]
  /// @param[out] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[num_points][gdim][tdim]
  /// @param[out] detJ
  ///         (Pseudo-)Determinant of Jacobian.
  ///         Dimensions: detJ[num_points]
  /// @param[out] K
  ///         (Pseudo-)Inverse of Jacobian of coordinate field.
  ///         Dimensions: K[num_points][tdim][gdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] x
  ///         Physical coordinates.
  ///         Dimensions: x[num_points][gdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  /// @param[in] cell_orientation
  ///         Orientation of the cell, 1 means flipped w.r.t. reference cell.
  ///         Only relevant on manifolds (tdim < gdim).
  ///
  void (*compute_reference_geometry)(double* X, double* J, double* detJ,
                                     double* K, int64_t num_points,
                                     const double* x,
                                     const double* coordinate_dofs,
                                     int cell_orientation)
      = NULL;

  /// Compute Jacobian of coordinate mapping J = dx/dX at reference coordinates
  /// X
  ///
  /// @param[out] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[num_points][gdim][tdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] X
  ///         Reference cell coordinates.
  ///         Dimensions: X[num_points][tdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  ///
  void (*compute_jacobians)(double* J, int64_t num_points, const double* X,
                            const double* coordinate_dofs)
      = NULL;

  /// Compute determinants of (pseudo-)Jacobians J
  ///
  /// @param[out] detJ
  ///         (Pseudo-)Determinant of Jacobian.
  ///         Dimensions: detJ[num_points]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[num_points][gdim][tdim]
  /// @param[in] cell_orientation
  ///         Orientation of the cell, 1 means flipped w.r.t. reference cell.
  ///         Only relevant on manifolds (tdim < gdim).
  ///
  void (*compute_jacobian_determinants)(double* detJ, int64_t num_points,
                                        const double* J, int cell_orientation)
      = NULL;

  /// Compute (pseudo-)inverses K of (pseudo-)Jacobians J
  ///
  /// @param[out] K
  ///         (Pseudo-)Inverse of Jacobian of coordinate field.
  ///         Dimensions: K[num_points][tdim][gdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[num_points][gdim][tdim]
  /// @param[in] detJ
  ///         (Pseudo-)Determinant of Jacobian.
  ///         Dimensions: detJ[num_points]
  ///
  void (*compute_jacobian_inverses)(double* K, int64_t num_points,
                                    const double* J, const double* detJ)
      = NULL;

  /// Combined (for convenience) computation of x, J, detJ, K from X and
  /// coordinate_dofs on a cell
  ///
  /// @param[out] x
  ///         Physical coordinates.
  ///         Dimensions: x[num_points][gdim]
  /// @param[out] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[num_points][gdim][tdim]
  /// @param[out] detJ
  ///         (Pseudo-)Determinant of Jacobian.
  ///         Dimensions: detJ[num_points]
  /// @param[out] K
  ///         (Pseudo-)Inverse of Jacobian of coordinate field.
  ///         Dimensions: K[num_points][tdim][gdim]
  /// @param[in] num_points
  ///         Number of points.
  /// @param[in] X
  ///         Reference cell coordinates.
  ///         Dimensions: X[num_points][tdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  /// @param[in] cell_orientation
  ///         Orientation of the cell, 1 means flipped w.r.t. reference cell.
  ///         Only relevant on manifolds (tdim < gdim).
  ///
  void (*compute_geometry)(double* x, double* J, double* detJ, double* K,
                           int64_t num_points, const double* X,
                           const double* coordinate_dofs, int cell_orientation)
      = NULL;

  /// Compute x and J at midpoint of cell
  ///
  /// @param[out] x
  ///         Physical coordinates.
  ///         Dimensions: x[gdim]
  /// @param[out] J
  ///         Jacobian of coordinate field, J = dx/dX.
  ///         Dimensions: J[gdim][tdim]
  /// @param[in] coordinate_dofs
  ///         Dofs of the coordinate field on the cell.
  ///         Dimensions: coordinate_dofs[num_dofs][gdim].
  ///
  void (*compute_midpoint_geometry)(double* x, double* J,
                                    const double* coordinate_dofs)
      = NULL;

} ufc_coordinate_mapping;

// FIXME: Is this required for integrals?
// Number of coefficients
// int64_t num_coefficients() const = 0;

typedef struct ufc_cell_integral
{
  const bool* enabled_coefficients = NULL;
  int64_t num_cells = -1;
  void (*tabulate_tensor)(double* A, const double* const* w,
                          const double* coordinate_dofs, int cell_orientation)
      = NULL;
} ufc_cell_integral;

typedef struct ufc_exterior_facet_integral
{
  const bool* enabled_coefficients = NULL;
  int64_t num_cells = -1;
  void (*tabulate_tensor)(double* A, const double* const* w,
                          const double* coordinate_dofs, int64_t facet,
                          int cell_orientation)
      = NULL;
} ufc_exterior_facet_integral;

typedef struct ufc_interior_facet_integral
{
  const bool* enabled_coefficients = NULL;
  int64_t num_cells = -1;
  void (*tabulate_tensor)(double* A, const double* const* w,
                          const double* coordinate_dofs_0,
                          const double* coordinate_dofs_1, int64_t facet_0,
                          int64_t facet_1, int cell_orientation_0,
                          int cell_orientation_1)
      = NULL;
} ufc_interior_facet_integral;

typedef struct ufc_vertex_integral
{
  const bool* enabled_coefficients = NULL;
  int64_t num_cells = -1;
  void (*tabulate_tensor)(double* A, const double* const* w,
                          const double* coordinate_dofs, int64_t vertex,
                          int cell_orientation)
      = NULL;
} ufc_vertex_integral;

typedef struct ufc_custom_integral
{
  const bool* enabled_coefficients = NULL;
  int64_t num_cells = -1;
  void (*tabulate_tensor)(double* A, const double* const* w,
                          const double* coordinate_dofs,
                          int64_t num_quadrature_points,
                          const double* quadrature_points,
                          const double* quadrature_weights,
                          const double* facet_normals, int cell_orientation)
      = NULL;
} ufc_custom_integral;

/// This class defines the interface for the assembly of the global
/// tensor corresponding to a form with r + n arguments, that is, a
/// mapping
///
///     a : V1 x V2 x ... Vr x W1 x W2 x ... x Wn -> R
///
/// with arguments v1, v2, ..., vr, w1, w2, ..., wn. The rank r
/// global tensor A is defined by
///
///     A = a(V1, V2, ..., Vr, w1, w2, ..., wn),
///
/// where each argument Vj represents the application to the
/// sequence of basis functions of Vj and w1, w2, ..., wn are given
/// fixed functions (coefficients).
typedef struct ufc_form
{
  /// String identifying the form
  const char* signature = NULL;

  /// Rank of the global tensor (r)
  int64_t rank = -1;

  /// Number of coefficients (n)
  int64_t num_coefficients = -1;

  /// Return original coefficient position for each coefficient
  ///
  /// @param i
  ///        Coefficient number, 0 <= i < n
  ///
  int64_t (*original_coefficient_position)(int64_t i) = NULL;

  /// Create a new finite element for parameterization of coordinates
  ufc_finite_element* (*create_coordinate_finite_element)() = NULL;

  /// Create a new dofmap for parameterization of coordinates
  ufc_dofmap* (*create_coordinate_dofmap)() = NULL;

  /// Create a new coordinate mapping
  ufc_coordinate_mapping* (*create_coordinate_mapping)() = NULL;

  /// Create a new finite element for argument function 0 <= i < r+n
  ///
  /// @param i
  ///        Argument number if 0 <= i < r
  ///        Coefficient number j=i-r if r+j <= i < r+n
  ///
  ufc_finite_element* (*create_finite_element)(int64_t i) = NULL;

  /// Create a new dofmap for argument function 0 <= i < r+n
  ///
  /// @param i
  ///        Argument number if 0 <= i < r
  ///        Coefficient number j=i-r if r+j <= i < r+n
  ///
  ufc_dofmap* (*create_dofmap)(int64_t i) = NULL;

  /// Upper bound on subdomain ids for cell integrals
  int64_t max_cell_subdomain_id = -1;

  /// Upper bound on subdomain ids for exterior facet integrals
  int64_t max_exterior_facet_subdomain_id = -1;

  /// Upper bound on subdomain ids for interior facet integrals
  int64_t max_interior_facet_subdomain_id = -1;

  /// Upper bound on subdomain ids for vertex integrals
  int64_t max_vertex_subdomain_id = -1;

  /// Upper bound on subdomain ids for custom integrals
  int64_t max_custom_subdomain_id = -1;

  /// Whether form has any cell integrals
  bool has_cell_integrals;

  /// Whether form has any exterior facet integrals
  bool has_exterior_facet_integrals;

  /// Whether form has any interior facet integrals
  bool has_interior_facet_integrals;

  /// Whether form has any vertex integrals
  bool has_vertex_integrals;

  /// Whether form has any custom integrals
  bool has_custom_integrals;

  /// Create a new cell integral on sub domain subdomain_id
  ufc_cell_integral* (*create_cell_integral)(int64_t subdomain_id) = NULL;

  /// Create a new exterior facet integral on sub domain subdomain_id
  ufc_exterior_facet_integral* (*create_exterior_facet_integral)(
      int64_t subdomain_id)
      = NULL;

  /// Create a new interior facet integral on sub domain subdomain_id
  ufc_interior_facet_integral* (*create_interior_facet_integral)(
      int64_t subdomain_id)
      = NULL;

  /// Create a new vertex integral on sub domain subdomain_id
  ufc_vertex_integral* (*create_vertex_integral)(int64_t subdomain_id) = NULL;

  /// Create a new custom integral on sub domain subdomain_id
  ufc_custom_integral* (*create_custom_integral)(int64_t subdomain_id) = NULL;

  /// Create a new cell integral on everywhere else
  ufc_cell_integral* (*create_default_cell_integral)() = NULL;

  /// Create a new exterior facet integral on everywhere else
  ufc_exterior_facet_integral* (*create_default_exterior_facet_integral)()
      = NULL;

  /// Create a new interior facet integral on everywhere else
  ufc_interior_facet_integral* (*create_default_interior_facet_integral)()
      = NULL;

  /// Create a new vertex integral on everywhere else
  ufc_vertex_integral* (*create_default_vertex_integral)() = NULL;

  /// Create a new custom integral on everywhere else
  ufc_custom_integral* (*create_default_custom_integral)() = NULL;
} ufc_form;

struct dolfin_function_space
{
  // Pointer to factory function that creates a new ufc_finite_element
  ufc_finite_element* (*element)(void);

  // Pointer to factory function that creates a new ufc_dofmap
  ufc_dofmap* (*dofmap)(void);

  // Pointer to factory function that creates a new ufc_coordinate_mapping
  ufc_coordinate_mapping* (*coordinate_mapping)(void);
};

struct dolfin_form
{
  // Pointer to factory function that returns a new ufc_form
  ufc_form* (*form)(void);

  // Pointer to function that returns name of coefficient i
  const char* (*coefficient_name_map)(int i);

  // Pointer to function that returns index of coefficient
  int (*coefficient_number_map)(const char* name);
};

#endif
