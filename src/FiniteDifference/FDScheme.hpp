/*
 * FiniteDifferences.hpp
 *
 *  Created on: Sep 17, 2015
 *      Author: i-bird
 */

#ifndef OPENFPM_NUMERICS_SRC_FINITEDIFFERENCE_FDSCHEME_HPP_
#define OPENFPM_NUMERICS_SRC_FINITEDIFFERENCE_FDSCHEME_HPP_

#include "../Matrix/SparseMatrix.hpp"
#include "Grid/grid_dist_id.hpp"
#include "Grid/grid_dist_id_iterator_sub.hpp"
#include "eq.hpp"
#include "data_type/scalar.hpp"
#include "NN/CellList/CellDecomposer.hpp"
#include "Grid/staggered_dist_grid_util.hpp"

/*! \brief Finite Differences
 *
 * This class is able to discreatize on a Matrix any system of equations producing a linear system of type \f$Ax=B\f$. In order to create a consistent
 * Matrix it is required that each processor must contain a contiguos range on grid points without
 * holes. In order to ensure this, each processor produce a contiguos local labelling of its local
 * points. Each processor also add an offset equal to the number of local
 * points of the processors with id smaller than him, to produce a global and non overlapping
 * labelling. An example is shown in the figures down, here we have
 * a grid 8x6 divided across two processor each processor label locally its grid points
 *
 * \verbatim
 *
+--------------------------+
| 1   2   3   4| 1  2  3  4|
|              |           |
| 5   6   7   8| 5  6  7  8|
|              |           |
| 9  10  11  12| 9 10 11 12|
+--------------------------+
|13  14  15| 13 14 15 16 17|
|          |               |
|16  17  18| 18 19 20 21 22|
|          |               |
|19  20  21| 23 24 25 26 27|
+--------------------------+

 *
 *
 * \endverbatim
 *
 * To the local relabelling is added an offset to make the local id global and non overlapping
 *
 *
 * \verbatim
 *
+--------------------------+
| 1   2   3   4|23 24 25 26|
|              |           |
| 5   6   7   8|27 28 29 30|
|              |           |
| 9  10  12  13|31 32 33 34|
+--------------------------+
|14  15  16| 35 36 37 38 39|
|          |               |
|17  18  19| 40 41 42 43 44|
|          |               |
|20  21  22| 45 46 47 48 49|
+--------------------------+
 *
 *
 * \endverbatim
 *
 * \tparam Sys_eqs Definition of the system of equations
 *
 * # Examples
 *
 * ## Solve lid-driven cavity 2D for incompressible fluid (inertia=0 --> Re=0)
 *
 * In this case the system of equation to solve is
 *
 * \f$
\left\{
\begin{array}{c}
\eta\nabla v_x + \partial_x P = 0 \quad Eq1 \\
\eta\nabla v_y + \partial_y P = 0 \quad Eq2 \\
\partial_x v_x + \partial_y v_y = 0 \quad Eq3
\end{array}
\right.  \f$

and  boundary conditions

 * \f$
\left\{
\begin{array}{c}
v_x = 0, v_y = 0 \quad x = 0 \quad B1\\
v_x = 0, v_y = 1.0 \quad x = L \quad B2\\
v_x = 0, v_y = 0 \quad y = 0 \quad B3\\
v_x = 0, v_y = 0 \quad y = L \quad B4\\
\end{array}
\right.  \f$

 *
 * with \f$v_x\f$ and \f$v_y\f$ the velocity in x and y and \f$P\f$ Pressure
 *
 * In order to solve such system first we define the general properties of the system
 *
 *	\snippet eq_unit_test.hpp Definition of the system
 *
 * ## Define the equations of the system
 *
 * \snippet eq_unit_test.hpp Definition of the equation of the system in the bulk and at the boundary
 *
 * ## Define the domain and impose the equations
 *
 * \snippet eq_unit_test.hpp lid-driven cavity 2D
 *
 * # 3D
 *
 * A 3D case is given in the examples
 *
 */

template<typename Sys_eqs>
class FDScheme
{
public:

	// Distributed grid map
	typedef grid_dist_id<Sys_eqs::dims,typename Sys_eqs::stype,scalar<size_t>,typename Sys_eqs::b_grid::decomposition> g_map_type;

	typedef Sys_eqs Sys_eqs_typ;

private:

	// Padding
	Padding<Sys_eqs::dims> pd;

	typedef typename Sys_eqs::SparseMatrix_type::triplet_type triplet;

	openfpm::vector<typename Sys_eqs::stype> b;

	// Domain Grid informations
	const grid_sm<Sys_eqs::dims,void> & gs;

	// Get the grid spacing
	typename Sys_eqs::stype spacing[Sys_eqs::dims];

	// mapping grid
	g_map_type g_map;

	// row of the matrix
	size_t row;

	// row on b
	size_t row_b;

	// Grid points that has each processor
	openfpm::vector<size_t> pnt;

	// Staggered position for each property
	comb<Sys_eqs::dims> s_pos[Sys_eqs::nvar];

	// Each point in the grid has a global id, to decompose correctly the Matrix each processor contain a
	// contiguos range of global id, example processor 0 can have from 0 to 234 and processor 1 from 235 to 512
	// no processors can have holes in the sequence, this number indicate where the sequence start for this
	// processor
	size_t s_pnt;

	struct key_and_eq
	{
		grid_key_dx<Sys_eqs::dims> key;
		size_t eq;
	};

	/*! \brief From the row Matrix position to the spatial position
	 *
	 * \param row Matrix
	 *
	 * \return spatial position
	 *
	 */
	inline key_and_eq from_row_to_key(size_t row)
	{
		key_and_eq ke;
		auto it = g_map.getDomainIterator();

		while (it.isNext())
		{
			size_t row_low = g_map.template get<0>(it.get());

			if (row >= row_low * Sys_eqs::nvar && row < row_low * Sys_eqs::nvar + Sys_eqs::nvar)
			{
				ke.eq = row - row_low * Sys_eqs::nvar;
				ke.key = g_map.getGKey(it.get());
				ke.key -= pd.getKP1();
				return ke;
			}

			++it;
		}
		std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " the row does not map to any position" << "\n";

		return ke;
	}

	/* \brief calculate the mapping grid size with padding
	 *
	 * \param gs original grid size
	 *
	 * \return padded grid size
	 *
	 */
	inline const std::vector<size_t> padding( const size_t (& sz)[Sys_eqs::dims], Padding<Sys_eqs::dims> & pd)
	{
		std::vector<size_t> g_sz_pad(Sys_eqs::dims);

		for (size_t i = 0 ; i < Sys_eqs::dims ; i++)
			g_sz_pad[i] = sz[i] + pd.getLow(i) + pd.getHigh(i);

		return g_sz_pad;
	}

	/*! \brief Check if the Matrix is consistent
	 *
	 */
	void consistency()
	{
		openfpm::vector<triplet> & trpl = A.getMatrixTriplets();

		// A and B must have the same rows
		if (row != row_b)
			std::cerr << "Error " << __FILE__ << ":" << __LINE__ << "the term B and the Matrix A for Ax=B must contain the same number of rows\n";

		// Indicate all the non zero rows
		openfpm::vector<unsigned char> nz_rows;
		nz_rows.resize(row_b);

		for (size_t i = 0 ; i < trpl.size() ; i++)
		{
			nz_rows.get(trpl.get(i).row()) = true;
		}

		// Indicate all the non zero colums
		openfpm::vector<unsigned> nz_cols;
		nz_cols.resize(row_b);

		for (size_t i = 0 ; i < trpl.size() ; i++)
		{
			nz_cols.get(trpl.get(i).col()) = true;
		}

		// all the rows must have a non zero element
		for (size_t i = 0 ; i < nz_rows.size() ; i++)
		{
			if (nz_rows.get(i) == false)
			{
				key_and_eq ke = from_row_to_key(i);
				std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " Ill posed matrix row " << i <<  " is not filled, position " << ke.key.to_string() << " equation: " << ke.eq << "\n";
			}
		}

		// all the colums must have a non zero element
		for (size_t i = 0 ; i < nz_cols.size() ; i++)
		{
			if (nz_cols.get(i) == false)
				std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " Ill posed matrix colum " << i << " is not filled\n";
		}
	}

public:

	/*! \brief set the staggered position for each property
	 *
	 * \param vector containing the staggered position for each property
	 *
	 */
	void setStagPos(comb<Sys_eqs::dims> (& sp)[Sys_eqs::nvar])
	{
		for (size_t i = 0 ; i < Sys_eqs::nvar ; i++)
			s_pos[i] = sp[i];
	}

	/*! \brief compute the staggered position for each property
	 *
	 * This is compute from the value_type stored by Sys_eqs::b_grid::value_type
	 *
	 * ### Example
	 *
	 * \snippet eq_unit_test.hpp Compute staggered properties
	 *
	 */
	void computeStag()
	{
		typedef typename Sys_eqs::b_grid::value_type prp_type;

		openfpm::vector<comb<Sys_eqs::dims>> c_prp[prp_type::max_prop];

		stag_set_position<Sys_eqs::dims,prp_type> ssp(c_prp);

		boost::mpl::for_each_ref< boost::mpl::range_c<int,0,prp_type::max_prop> >(ssp);
	}

	/*! \brief Get the specified padding
	 *
	 * \return the padding specified
	 *
	 */
	const Padding<Sys_eqs::dims> & getPadding()
	{
		return pd;
	}

	/*! \brief Return the map between the grid index position and the position in the distributed vector
	 *
	 * It is the map explained in the intro of the FDScheme
	 *
	 * \return the map
	 *
	 */
	const g_map_type & getMap()
	{
		return g_map;
	}

	/*! \brief Constructor
	 *
	 * \param pd Padding, how many points out of boundary are present
	 * \param domain extension of the domain
	 * \param gs grid infos where Finite differences work
	 * \param dec Decomposition of the domain
	 *
	 */
	FDScheme(Padding<Sys_eqs::dims> & pd, const Box<Sys_eqs::dims,typename Sys_eqs::stype> & domain, const grid_sm<Sys_eqs::dims,void> & gs, const typename Sys_eqs::b_grid::decomposition & dec)
	:pd(pd),gs(gs),g_map(dec,padding(gs.getSize(),pd),domain,Ghost<Sys_eqs::dims,typename Sys_eqs::stype>(1)),row(0),row_b(0)
	{
		Vcluster & v_cl = dec.getVC();

		// Calculate the size of the local domain
		size_t sz = g_map.getLocalDomainSize();

		// Get the total size of the local grids on each processors
		v_cl.allGather(sz,pnt);
		v_cl.execute();
		s_pnt = 0;

		// calculate the starting point for this processor
		for (size_t i = 0 ; i < v_cl.getProcessUnitID() ; i++)
			s_pnt += pnt.get(i);

		// Calculate the starting point

		// Counter
		size_t cnt = 0;

		// Create the re-mapping grid
		auto it = g_map.getDomainIterator();

		while (it.isNext())
		{
			auto key = it.get();

			g_map.template get<0>(key) = cnt + s_pnt;

			++cnt;
			++it;
		}

		// sync the ghost

		g_map.template ghost_get<0>();

		// Create a CellDecomposer and calculate the spacing

		size_t sz_g[Sys_eqs::dims];
		for (size_t i = 0 ; i < Sys_eqs::dims ; i++)
			sz_g[i] = gs.getSize()[i] - 1;

		CellDecomposer_sm<Sys_eqs::dims,typename Sys_eqs::stype> cd(domain,sz_g,0);

		for (size_t i = 0 ; i < Sys_eqs::dims ; i++)
			spacing[i] = cd.getCellBox().getHigh(i);
	}

	/*! \brief Impose an operator
	 *
	 * This function impose an operator on a box region to produce the system
	 *
	 * Ax = b
	 *
	 * ## Stokes equation, lid driven cavity with one splipping wall
	 *
	 * \param op Operator to impose (A term)
	 * \param num right hand side of the term (b term)
	 * \param id Equation id in the system that we are imposing
	 * \param start starting point of the box
	 * \param stop stop point of the box
	 *
	 */
	template<typename T> void impose(const T & op , typename Sys_eqs::stype num ,long int id ,const long int (& start)[Sys_eqs::dims], const long int (& stop)[Sys_eqs::dims], bool skip_first = false)
	{
		// add padding to start and stop
		grid_key_dx<Sys_eqs::dims> start_k = grid_key_dx<Sys_eqs::dims>(start) + pd.getKP1();
		grid_key_dx<Sys_eqs::dims> stop_k = grid_key_dx<Sys_eqs::dims>(stop) + pd.getKP1();

		auto it = g_map.getSubDomainIterator(start_k,stop_k);

		impose(op,num,id,it,skip_first);
	}

	/*! \brief Impose an operator
	 *
	 * This function impose an operator on a particular grid region to produce the system
	 *
	 * Ax = b
	 *
	 * ## Stokes equation, lid driven cavity with one splipping wall
	 *
	 * \param op Operator to impose (A term)
	 * \param num right hand side of the term (b term)
	 * \param id Equation id in the system that we are imposing
	 * \param it_d iterator that define where you want to impose
	 *
	 */
	template<typename T> void impose(const T & op , typename Sys_eqs::stype num ,long int id ,grid_dist_iterator_sub<Sys_eqs::dims,typename g_map_type::d_grid> it_d, bool skip_first = false)
	{
		openfpm::vector<triplet> & trpl = A.getMatrixTriplets();

		auto it = it_d;
		grid_sm<Sys_eqs::dims,void> gs = g_map.getGridInfoVoid();

		std::unordered_map<long int,float> cols;

		// resize b if needed
		b.resize(Sys_eqs::nvar * gs.size());

		bool is_first = skip_first;

		// iterate all the grid points
		while (it.isNext())
		{
			if (is_first == true)
			{
				++it;
				is_first = false;
				continue;
			}

			// get the position
			auto key = it.get();

			// Calculate the non-zero colums
			T::value(g_map,key,gs,spacing,cols,1.0);

			// create the triplet

			for ( auto it = cols.begin(); it != cols.end(); ++it )
			{
				trpl.add();
				trpl.last().row() = g_map.template get<0>(key)*Sys_eqs::nvar + id;
				trpl.last().col() = it->first;
				trpl.last().value() = it->second;

//				std::cout << "(" << trpl.last().row() << "," << trpl.last().col() << "," << trpl.last().value() << ")" << "\n";
			}

			b.get(g_map.template get<0>(key)*Sys_eqs::nvar + id) = num;

			cols.clear();
//			std::cout << "\n";

			// if SE_CLASS1 is defined check the position
#ifdef SE_CLASS1
			T::position(gkey,gs,s_pos);
#endif

			++row;
			++row_b;
			++it;
		}
	}

	typename Sys_eqs::SparseMatrix_type A;

	/*! \brief produce the Matrix
	 *
	 *  \return the Sparse matrix produced
	 *
	 */
	typename Sys_eqs::SparseMatrix_type & getA()
	{
#ifdef SE_CLASS1
		consistency();
#endif
		A.resize(row,row);

		return A;

	}

	typename Sys_eqs::Vector_type B;

	/*! \brief produce the B vector
	 *
	 *  \return the vector produced
	 *
	 */
	typename Sys_eqs::Vector_type & getB()
	{
#ifdef SE_CLASS1
		consistency();
#endif

		B.resize(row_b);

		// copy the vector
		for (size_t i = 0; i < row_b; i++)
			B.get(i) = b.get(i);

		return B;
	}
};

#define EQS_FIELDS 0
#define EQS_SPACE 1


#endif /* OPENFPM_NUMERICS_SRC_FINITEDIFFERENCE_FDSCHEME_HPP_ */