//
// Created by Abhinav Singh on 15.06.20.
//

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 40

#include "config.h"

#define BOOST_TEST_DYN_LINK


#include "util/util_debug.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include "DCPSE_op.hpp"
#include "DCPSE_Solver.hpp"
#include "Operators/Vector/vector_dist_operators.hpp"
#include "Vector/vector_dist_subset.hpp"
#include "EqnsStruct.hpp"
#include "Decomposition/Distribution/SpaceDistribution.hpp"


BOOST_AUTO_TEST_SUITE(dcpse_op_suite_tests3)

    BOOST_AUTO_TEST_CASE(Active3dSimple) {
        timer tt2;
        tt2.start();
        size_t grd_sz = 21;
        double dt = 1e-3;
        double boxsize = 10;
        const size_t sz[3] = {grd_sz, grd_sz, grd_sz};
        Box<3, double> box({0, 0, 0}, {boxsize, boxsize, boxsize});
        size_t bc[3] = {NON_PERIODIC, NON_PERIODIC, NON_PERIODIC};
        double Lx = box.getHigh(0);
        double Ly = box.getHigh(1);
        double Lz = box.getHigh(2);
        double spacing = box.getHigh(0) / (sz[0] - 1);
        double rCut = 3.9 * spacing;
        double rCut2 = 3.9 * spacing;
        int ord = 2;
        int ord2 = 2;
        double sampling_factor = 4.0;
        double sampling_factor2 = 2.4;
        Ghost<3, double> ghost(rCut);
        auto &v_cl = create_vcluster();
        /*                                 pol                          V           vort              Ext                 Press     strain       stress                      Mfield,   dPol                      dV         RHS                  f1     f2     f3    f4     f5     f6       H               V_t      div    H_t                                                                                      delmu */
        vector_dist<3, double, aggregate<VectorS<3, double>, VectorS<3, double>, double[3][3], VectorS<3, double>, double, double[3][3], double[3][3], VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, double, double, double, double, double, double, double, VectorS<3, double>, double, double, double[3], double[3], double[3], double[3], double[3], double[3], double, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, double, double, double, double[3][3],double[3][3]>> Particles(
                0, box, bc, ghost);


        double x0, y0, z0, x1, y1, z1;
        x0 = box.getLow(0);
        y0 = box.getLow(1);
        z0 = box.getLow(2);
        x1 = box.getHigh(0);
        y1 = box.getHigh(1);
        z1 = box.getHigh(2);

        auto it = Particles.getGridIterator(sz);
        while (it.isNext()) {
            Particles.add();
            auto key = it.get();
            double x = key.get(0) * it.getSpacing(0);
            Particles.getLastPos()[0] = x;
            double y = key.get(1) * it.getSpacing(1);
            Particles.getLastPos()[1] = y;
            double z = key.get(2) * it.getSpacing(1);
            Particles.getLastPos()[2] = z;
            ++it;
        }

        Particles.map();
        Particles.ghost_get<0>();

        openfpm::vector<aggregate<int>> bulk;
        openfpm::vector<aggregate<int>> Surface;
        openfpm::vector<aggregate<int>> Surface_without_corners;
        openfpm::vector<aggregate<int>> f_ul;
        openfpm::vector<aggregate<int>> f_ur;
        openfpm::vector<aggregate<int>> f_dl;
        openfpm::vector<aggregate<int>> f_dr;

        openfpm::vector<aggregate<int>> b_ul;
        openfpm::vector<aggregate<int>> b_ur;
        openfpm::vector<aggregate<int>> b_dl;
        openfpm::vector<aggregate<int>> b_dr;


        constexpr int x = 0;
        constexpr int y = 1;
        constexpr int z = 2;


        constexpr int Polarization = 0;
        constexpr int Velocity = 1;
        constexpr int Vorticity = 2;
        constexpr int ExtForce = 3;
        constexpr int Pressure = 4;
        constexpr int Strain_rate = 5;
        constexpr int Stress = 6;
        constexpr int MolField = 7;
        auto Pos = getV<PROP_POS>(Particles);

        auto Pol = getV<Polarization>(Particles);
        auto V = getV<Velocity>(Particles);
        auto W = getV<Vorticity>(Particles);
        auto g = getV<ExtForce>(Particles);
        auto P = getV<Pressure>(Particles);
        auto u = getV<Strain_rate>(Particles);
        auto sigma = getV<Stress>(Particles);
        auto h = getV<MolField>(Particles);
        auto dPol = getV<8>(Particles);
        auto dV = getV<9>(Particles);
        auto RHS = getV<10>(Particles);
        auto f1 = getV<11>(Particles);
        auto f2 = getV<12>(Particles);
        auto f3 = getV<13>(Particles);
        auto f4 = getV<14>(Particles);
        auto f5 = getV<15>(Particles);
        auto f6 = getV<16>(Particles);
        auto H = getV<17>(Particles);
        auto V_t = getV<18>(Particles);
        auto div = getV<19>(Particles);
        auto H_t = getV<20>(Particles);
        auto Df1 = getV<21>(Particles);
        auto Df2 = getV<22>(Particles);
        auto Df3 = getV<23>(Particles);
        auto Df4 = getV<24>(Particles);
        auto Df5 = getV<25>(Particles);
        auto Df6 = getV<26>(Particles);
        auto delmu = getV<27>(Particles);
        auto k1 = getV<28>(Particles);
        auto k2 = getV<29>(Particles);
        auto k3 = getV<30>(Particles);
        auto k4 = getV<31>(Particles);
        auto H_p_b = getV<32>(Particles);
        auto FranckEnergyDensity = getV<33>(Particles);
        auto r = getV<34>(Particles);
        auto q = getV<35>(Particles);


        double eta = 1.0;
        double nu = -0.5;
        double gama = 0.1;
        double zeta = 0.07;
        double Ks = 1.0;
        double Kt = 1.1;
        double Kb = 1.5;
        double lambda = 0.1;
        //double delmu = -1.0;
        g = 0;
        delmu = -1.0;
        P = 0;
        V = 0;
        // Here fill up the boxes for particle boundary detection.
        Particles.ghost_get<ExtForce, 27>(SKIP_LABELLING);

        // Here fill up the boxes for particle detection.

        Box<3, double> up(
                {box.getLow(0) - spacing / 2.0, box.getHigh(1) - spacing / 2.0, box.getLow(2) - spacing / 2.0},
                {box.getHigh(0) + spacing / 2.0, box.getHigh(1) + spacing / 2.0, box.getHigh(2) + spacing / 2.0});

        Box<3, double> down(
                {box.getLow(0) - spacing / 2.0, box.getLow(1) - spacing / 2.0, box.getLow(2) - spacing / 2.0},
                {box.getHigh(0) + spacing / 2.0, box.getLow(1) + spacing / 2.0, box.getHigh(2) + spacing / 2.0});

        Box<3, double> left(
                {box.getLow(0) - spacing / 2.0, box.getLow(1) - spacing / 2.0, box.getLow(2) - spacing / 2.0},
                {box.getLow(0) + spacing / 2.0, box.getHigh(1) + spacing / 2.0, box.getHigh(2) + spacing / 2.0});

        Box<3, double> right(
                {box.getHigh(0) - spacing / 2.0, box.getLow(1) - spacing / 2.0, box.getLow(2) - spacing / 2.0},
                {box.getHigh(0) + spacing / 2.0, box.getHigh(1) + spacing / 2.0, box.getHigh(2) + spacing / 2.0});

        Box<3, double> front(
                {box.getLow(0) - spacing / 2.0, box.getLow(1) - spacing / 2.0, box.getLow(2) - spacing / 2.0},
                {box.getHigh(0) + spacing / 2.0, box.getHigh(1) + spacing / 2.0, box.getLow(2) + spacing / 2.0});

        Box<3, double> back(
                {box.getLow(0) - spacing / 2.0, box.getLow(1) - spacing / 2.0, box.getHigh(2) - spacing / 2.0},
                {box.getHigh(0) + spacing / 2.0, box.getHigh(1) + spacing / 2.0, box.getHigh(2) + spacing / 2.0});

        openfpm::vector<Box<3, double>> boxes;
        boxes.add(up);
        boxes.add(down);
        boxes.add(left);
        boxes.add(right);
        boxes.add(front);
        boxes.add(back);
        VTKWriter<openfpm::vector<Box<3, double>>, VECTOR_BOX> vtk_box;
        vtk_box.add(boxes);
        vtk_box.write("boxes_3d.vtk");
        auto it2 = Particles.getDomainIterator();

        while (it2.isNext()) {
            auto p = it2.get();
            Point<3, double> xp = Particles.getPos(p);
            Particles.getProp<0>(p)[x] = sin(2 * M_PI * (cos((2 * xp[x] - Lx) / Lx) - sin((2 * xp[y] - Ly) / Ly)));
            Particles.getProp<0>(p)[y] = cos(2 * M_PI * (cos((2 * xp[x] - Lx) / Lx) - sin((2 * xp[y] - Ly) / Ly)));
            Particles.getProp<0>(p)[z] = 0;
            if (front.isInside(xp) == true) {
                if (up.isInside(xp) == true) {
                    if (left.isInside(xp) == true) {
                        f_ul.add();
                        f_ul.last().get<0>() = p.getKey();
                    } else if (right.isInside(xp) == true) {
                        f_ur.add();
                        f_ur.last().get<0>() = p.getKey();
                    } else {
                        Surface_without_corners.add();
                        Surface_without_corners.last().get<0>() = p.getKey();
                    }
                } else if (down.isInside(xp) == true) {
                    if (left.isInside(xp) == true) {
                        f_dl.add();
                        f_dl.last().get<0>() = p.getKey();
                    }
                    if (right.isInside(xp) == true) {
                        f_dr.add();
                        f_dr.last().get<0>() = p.getKey();
                    } else {
                        Surface_without_corners.add();
                        Surface_without_corners.last().get<0>() = p.getKey();
                    }
                }
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else if (back.isInside(xp) == true) {
                if (up.isInside(xp) == true) {
                    if (left.isInside(xp) == true) {
                        b_ul.add();
                        b_ul.last().get<0>() = p.getKey();
                    } else if (right.isInside(xp) == true) {
                        b_ur.add();
                        b_ur.last().get<0>() = p.getKey();
                    } else {
                        Surface_without_corners.add();
                        Surface_without_corners.last().get<0>() = p.getKey();
                    }
                } else if (down.isInside(xp) == true) {
                    if (left.isInside(xp) == true) {
                        b_dl.add();
                        b_dl.last().get<0>() = p.getKey();
                    }
                    if (right.isInside(xp) == true) {
                        b_dr.add();
                        b_dr.last().get<0>() = p.getKey();
                    } else {
                        Surface_without_corners.add();
                        Surface_without_corners.last().get<0>() = p.getKey();
                    }
                }
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else if (left.isInside(xp) == true) {
                Surface_without_corners.add();
                Surface_without_corners.last().get<0>() = p.getKey();
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else if (right.isInside(xp) == true) {
                Surface_without_corners.add();
                Surface_without_corners.last().get<0>() = p.getKey();
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else if (up.isInside(xp) == true) {
                Surface_without_corners.add();
                Surface_without_corners.last().get<0>() = p.getKey();
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else if (down.isInside(xp) == true) {
                Surface_without_corners.add();
                Surface_without_corners.last().get<0>() = p.getKey();
                Surface.add();
                Surface.last().get<0>() = p.getKey();
            } else {
                bulk.add();
                bulk.last().get<0>() = p.getKey();
            }
            ++it2;
        }

        vector_dist_subset<3, double, aggregate<VectorS<3, double>, VectorS<3, double>, double[3][3], VectorS<3, double>, double, double[3][3], double[3][3], VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, double, double, double, double, double, double, double, VectorS<3, double>, double, double, double[3], double[3], double[3], double[3], double[3], double[3], double, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, VectorS<3, double>, double, double, double, double[3][3],double[3][3]>> Particles_subset(
                Particles, bulk);

        auto P_bulk = getV<Pressure>(Particles_subset);


        Particles.write_frame("Active3d_Parts", 0);

        //Particles_subset.write("Pars");
        Derivative_x Dx(Particles, ord, rCut, sampling_factor, support_options::RADIUS), Bulk_Dx(Particles_subset, ord,
                                                                                                 rCut, sampling_factor,
                                                                                                 support_options::RADIUS);
        Derivative_y Dy(Particles, ord, rCut, sampling_factor, support_options::RADIUS), Bulk_Dy(Particles_subset, ord,
                                                                                                 rCut, sampling_factor,
                                                                                                 support_options::RADIUS);
        Derivative_z Dz(Particles, ord, rCut, sampling_factor, support_options::RADIUS), Bulk_Dz(Particles_subset, ord,
                                                                                                 rCut, sampling_factor,
                                                                                                 support_options::RADIUS);
        Derivative_xy Dxy(Particles, ord, rCut2, sampling_factor2, support_options::RADIUS);
        Derivative_yz Dyz(Particles, ord, rCut2, sampling_factor2, support_options::RADIUS);
        Derivative_xz Dxz(Particles, ord, rCut2, sampling_factor2, support_options::RADIUS);
        auto Dyx = Dxy;
        auto Dzy = Dyz;
        auto Dzx = Dxz;

        Derivative_xx Dxx(Particles, ord, rCut2, sampling_factor2,
                          support_options::RADIUS);//, Dxx2(Particles, ord2, rCut2, sampling_factor2, support_options::RADIUS);
        Derivative_yy Dyy(Particles, ord, rCut2, sampling_factor2,
                          support_options::RADIUS);
        Derivative_zz Dzz(Particles, ord, rCut2, sampling_factor2,
                          support_options::RADIUS);

        V_t = V;


        eq_id vx, vy, vz;

        vx.setId(0);
        vy.setId(1);
        vz.setId(2);
        timer tt;
        double V_err_eps = 1e-3;
        double V_err = 1, V_err_old;
        int n = 0;
        int nmax = 300;
        int ctr = 0, errctr, Vreset = 0;
        double tim = 0;
        double tf = 1.024;
        div = 0;
        double sum, sum1, sum_k;
        while (tim <= tf) {
            tt.start();
            petsc_solver<double> solverPetsc;
            solverPetsc.setSolver(KSPGMRES);
            //solverPetsc.setRestart(250);
            solverPetsc.setPreconditioner(PCJACOBI);
            Particles.ghost_get<Polarization>(SKIP_LABELLING);

            auto divP = (Dx(Pol[x]) + Dy(Pol[y]) + Dz(Pol[z]));
            auto PcurlP = (Pol[x] * (Dy(Pol[z]) - Dz(Pol[y])) + Pol[y] * (Dz(Pol[x]) - Dx(Pol[z])) +
                           Pol[z] * (Dx(Pol[y]) - Dy(Pol[x])));
            auto temp1 = Pol[x] * (Dz(Pol[x]) - Dx(Pol[z])) + Pol[y] * (Dz(Pol[y]) - Dy(Pol[z]));
            auto temp2 = Pol[y] * (Dx(Pol[y]) - Dy(Pol[x])) + Pol[z] * (Dx(Pol[z]) - Dz(Pol[x]));
            auto temp3 = Pol[x] * (Dy(Pol[x]) - Dx(Pol[y])) + Pol[z] * (Dy(Pol[z]) - Dz(Pol[y]));
            auto PcrosscurlP_sq = temp1 * temp1 + temp2 * temp2 + temp3 * temp3;
            //auto temp4= Dx(Pol[x])*Dx(Pol[x]) + Dx(Pol[y])*Dx(Pol[y]) + Dx(Pol[z])*Dx(Pol[z]) - 2*Dx(Pol[y])*Dy(Pol[x]) + Dy(Pol[x])*Dy(Pol[x]) + 2*Dx(Pol[x])*Dy(Pol[y]) + Dy(Pol[y])*Dy(Pol[y]) + Dy(Pol[z])*Dy(Pol[z]) - 2*Dx(Pol[z])*Dz(Pol[x]) + Dz(Pol[x])*Dz(Pol[x]) - 2*Dy(Pol[z])*Dz(Pol[y]) + Dz(Pol[y])*Dz(Pol[y]) + 2*Dx(Pol[x])*Dz(Pol[z]) + 2*Dy(Pol[y])*Dz(Pol[z]) + Dz(Pol[z])*Dz(Pol[z]) + (Dxx(Pol[x]) + Dyy(Pol[x]) + Dzz(Pol[x]))*Pol[x] + (Dxx(Pol[y]) + Dyy(Pol[y]) + Dzz(Pol[y]))*Pol[y] + (Dxx(Pol[z]) + Dyy(Pol[z]) + Dzz(Pol[z]))*Pol[z];
            //f1[x]=2*(divP*divP+PcurlP*PcurlP+PcrosscurlP_sq-temp4;
            q[x][x] = Pol[x] * Pol[x];// - 1/3;
            q[y][y] = Pol[y] * Pol[y];// - 1/3;
            q[z][z] = Pol[z] * Pol[z];// - 1/3;
            q[x][y] = Pol[x] * Pol[y];
            q[y][z] = Pol[y] * Pol[z];
            q[x][z] = Pol[x] * Pol[z];
            //q[y][x] = q[x][y];
            //q[z][y] = q[y][z];
            //q[z][x] = q[x][z];

            f1 = 2 * (Dx(Pol[x]) * Dx(Pol[x]) + Dy(Pol[y]) * Dy(Pol[y]) + Dz(Pol[z]) * Dz(Pol[z]) +
                      Dx(Pol[y]) * Dx(Pol[y]) + Dx(Pol[z]) * Dx(Pol[z]) + Dy(Pol[x]) * Dy(Pol[x]) +
                      Dy(Pol[z]) * Dy(Pol[z]) + Dz(Pol[x]) * Dz(Pol[x]) + Dz(Pol[y]) * Dz(Pol[y]));
            f2 = divP * divP + PcrosscurlP_sq;
            f4 = -PcurlP;
            f6 = 2 * PcrosscurlP_sq - 1 / 3 * f1[x];
            FranckEnergyDensity = (-Ks + 3.0 * Kt + Kb) / 12.0 * f1 + (Ks - Kt) / 2.0 * f2 + (Kb - Ks) / 4.0 * f6;

            auto njdllqjx = Pol[x] * (Dxx(q[x][x]) + Dyy(q[x][x]) + Dzz(q[x][x])) +
                             Pol[y] * (Dxx(q[y][x]) + Dyy(q[y][x]) + Dzz(q[y][x])) +
                             Pol[z] * (Dxx(q[z][x]) + Dyy(q[z][x]) + Dzz(q[z][x]));
            auto njdllqjy = Pol[x] * (Dxx(q[x][y]) + Dyy(q[x][y]) + Dzz(q[x][y])) +
                             Pol[y] * (Dxx(q[y][y]) + Dyy(q[y][y]) + Dzz(q[y][y])) +
                             Pol[z] * (Dxx(q[z][y]) + Dyy(q[z][y]) + Dzz(q[z][y]));
            auto njdllqjz = Pol[x] * (Dxx(q[x][z]) + Dyy(q[x][z]) + Dzz(q[x][z])) +
                             Pol[y] * (Dxx(q[y][z]) + Dyy(q[y][z]) + Dzz(q[y][z])) +
                             Pol[z] * (Dxx(q[z][z]) + Dyy(q[z][z]) + Dzz(q[z][z]));

       /*     Df1[x] = -4 * njdllqjx;
            Df1[y] = -4 * njdllqjy;
            Df1[z] = -4 * njdllqjz;

            Df2[x] = -2 * (2 * Pol[x] * (Dxx(q[x][x]) + Dxy(q[x][y]) + Dxz(q[x][z])) + Pol[y] *
                                                                                       (Dxx(q[y][x]) + Dxy(q[y][y]) +
                                                                                        Dxz(q[y][z]) + Dyx(q[x][x]) +
                                                                                        Dyy(q[x][y]) + Dyz(q[x][z])) +
                           Pol[z] *
                           (Dxx(q[z][x]) + Dxy(q[z][y]) + Dxz(q[z][z]) + Dzx(q[x][x]) + Dzy(q[x][y]) + Dzz(q[x][z])));
            Df2[y] = -2 * (Pol[x] *
                           (Dyx(q[x][x]) + Dyy(q[x][y]) + Dyz(q[x][z]) + Dxx(q[y][x]) + Dxy(q[y][y]) + Dxz(q[y][z])) +
                           2 * Pol[y] * (Dyx(q[y][x]) + Dyy(q[y][y]) + Dyz(q[y][z])) + Pol[z] *
                                                                                       (Dyx(q[z][x]) + Dyy(q[z][y]) +
                                                                                        Dyz(q[z][z]) + Dzx(q[y][x]) +
                                                                                        Dzy(q[y][y]) + Dzz(q[y][z])));
            Df2[z] = -2 * (Pol[x] *
                           (Dzx(q[x][x]) + Dzy(q[x][y]) + Dzz(q[x][z]) + Dxx(q[z][x]) + Dxy(q[z][y]) + Dxz(q[z][z])) +
                           Pol[y] *
                           (Dzx(q[y][x]) + Dzy(q[y][y]) + Dzz(q[y][z]) + Dyx(q[z][x]) + Dyy(q[z][y]) + Dyz(q[z][z])) +
                           2 * Pol[z] * (Dzx(q[z][x]) + Dzy(q[z][y]) + Dzz(q[z][z])));*/
/*          auto txx=2*(Dx(q[x][x])*Dx(q[x][x])+Dx(q[x][y])*Dy(q[x][x])+Dx(q[x][z])*Dz(q[x][x])+Dy(q[y][x])*Dx(q[x][x])+Dy(q[y][y])*Dy(q[x][x])+Dy(q[y][z])*Dz(q[x][x])+Dz(q[z][x])*Dx(q[x][x])+Dz(q[z][y])*Dy(q[x][x])+Dz(q[z][z])*Dz(q[x][x])) + 2*(q[x][x]*Dxx(q[x][x])+q[x][x]*Dxx(q[x][x]))  +  ;
            auto txy=2*(Dx(q[x][x])*Dx(q[x][x])+Dx(q[x][y])*Dy(q[x][x])+Dx(q[x][z])*Dz(q[x][x])+Dy(q[y][x])*Dx(q[x][x])+Dy(q[y][y])*Dy(q[x][x])+Dy(q[y][z])*Dz(q[x][x])+Dz(q[z][x])*Dx(q[x][x])+Dz(q[z][y])*Dy(q[x][x])+Dz(q[z][z])*Dz(q[x][x])) +   +  ;
            auto txz=2*(Dx(q[x][x])*Dx(q[x][x])+Dx(q[x][y])*Dy(q[x][x])+Dx(q[x][z])*Dz(q[x][x])+Dy(q[y][x])*Dx(q[x][x])+Dy(q[y][y])*Dy(q[x][x])+Dy(q[y][z])*Dz(q[x][x])+Dz(q[z][x])*Dx(q[x][x])+Dz(q[z][y])*Dy(q[x][x])+Dz(q[z][z])*Dz(q[x][x])) +   +  ;
            auto tyx=;
            auto tyy=;
            auto tyz=2*(Dx(q[x][x])*Dx(q[x][x])+Dx(q[x][y])*Dy(q[x][x])+Dx(q[x][z])*Dz(q[x][x])+Dy(q[x][l])+Dz(q[x][l]);
            auto tzx=;
            auto tzy=2*(Dx(q[x][x])*Dx(q[x][x])+Dx(q[x][y])*Dy(q[x][x])+Dx(q[x][z])*Dz(q[x][x])+Dy(q[x][l])+Dz(q[x][l]);
            auto tzz=;*/

          /*  for (int i = 0; i <= 2; i += 1) {
                Df6[i]=0;
                for (int j = 0; i <= 2; i += 1) {
                    for (int k = 0; i <= 2; i += 1) {
                        for (int l = 0; i <= 2; i += 1) {
                            Df6[i] = Df6[i]+
                        }
                    }
                }
            }*/

        /*    Df6[x] = -2 * (Pol[x] * ()) + Pol[y] * () + Pol[z] * ();
            Df6[y] = -2 * (Pol[x] *
                           (Dyx(q[x][x]) + Dyy(q[x][y]) + Dyz(q[x][z]) + Dxx(q[y][x]) + Dxy(q[y][y]) + Dxz(q[y][z])) +
                           2 * Pol[y] * (Dyx(q[y][x]) + Dyy(q[y][y]) + Dyz(q[y][z])) + Pol[z] *
                                                                                       (Dyx(q[z][x]) + Dyy(q[z][y]) +
                                                                                        Dyz(q[z][z]) + Dzx(q[y][x]) +
                                                                                        Dzy(q[y][y]) + Dzz(q[y][z])));
            Df6[z] = -2 * (Pol[x] *
                           (Dzx(q[x][x]) + Dzy(q[x][y]) + Dzz(q[x][z]) + Dxx(q[z][x]) + Dxy(q[z][y]) + Dxz(q[z][z])) +
                           Pol[y] *
                           (Dzx(q[y][x]) + Dzy(q[y][y]) + Dzz(q[y][z]) + Dyx(q[z][x]) + Dyy(q[z][y]) + Dyz(q[z][z])) +
                           2 * Pol[z] * (Dzx(q[z][x]) + Dzy(q[z][y]) + Dzz(q[z][z])));*/




            //Particles.ghost_get<33>(SKIP_LABELLING);
            Particles.write("Polar_test");
            sigma[x][x] =
                    -Ks * Dx(Pol[x]) * Dx(Pol[x]) - Kb * Dx(Pol[y]) * Dx(Pol[y]) + (Kb - Ks) * Dy(Pol[x]) * Dx(Pol[y]);
            sigma[x][y] =
                    -Ks * Dy(Pol[y]) * Dx(Pol[y]) - Kb * Dy(Pol[x]) * Dx(Pol[x]) + (Kb - Ks) * Dx(Pol[y]) * Dx(Pol[x]);

            sigma[x][z] =
                    -Ks * Dy(Pol[z]) * Dx(Pol[z]) - Kb * Dy(Pol[x]) * Dx(Pol[x]) + (Kb - Ks) * Dx(Pol[z]) * Dx(Pol[x]);


            sigma[y][x] =
                    -Ks * Dx(Pol[x]) * Dy(Pol[x]) - Kb * Dx(Pol[y]) * Dy(Pol[y]) + (Kb - Ks) * Dy(Pol[x]) * Dy(Pol[y]);
            sigma[y][y] =
                    -Ks * Dy(Pol[y]) * Dy(Pol[y]) - Kb * Dy(Pol[x]) * Dy(Pol[x]) + (Kb - Ks) * Dx(Pol[y]) * Dy(Pol[x]);
            sigma[y][z] =
                    -Ks * Dx(Pol[z]) * Dy(Pol[z]) - Kb * Dx(Pol[y]) * Dy(Pol[y]) + (Kb - Ks) * Dy(Pol[z]) * Dy(Pol[y]);

            sigma[z][x] =
                    -Ks * Dx(Pol[z]) * Dx(Pol[z]) - Kb * Dx(Pol[y]) * Dx(Pol[y]) + (Kb - Ks) * Dy(Pol[x]) * Dx(Pol[y]);
            sigma[z][y] =
                    -Ks * Dy(Pol[y]) * Dx(Pol[y]) - Kb * Dy(Pol[x]) * Dx(Pol[x]) + (Kb - Ks) * Dx(Pol[y]) * Dx(Pol[x]);

            sigma[z][z] =
                    -Ks * Dx(Pol[x]) * Dy(Pol[x]) - Kb * Dx(Pol[y]) * Dy(Pol[y]) + (Kb - Ks) * Dy(Pol[x]) * Dy(Pol[y]);


            Particles.ghost_get<Stress>(SKIP_LABELLING);

            Particles.ghost_get<MolField>(SKIP_LABELLING);
            return;

            /*  f1 = gama * nu * Pol[x] * Pol[x] * (Pol[x] * Pol[x] - Pol[y] * Pol[y]) / (r);
              f2 = 2.0 * gama * nu * Pol[x] * Pol[y] * (Pol[x] * Pol[x] - Pol[y] * Pol[y]) / (r);
              f3 = gama * nu * Pol[y] * Pol[y] * (Pol[x] * Pol[x] - Pol[y] * Pol[y]) / (r);
              f4 = 2.0 * gama * nu * Pol[x] * Pol[x] * Pol[x] * Pol[y] / (r);
              f5 = 4.0 * gama * nu * Pol[x] * Pol[x] * Pol[y] * Pol[y] / (r);
              f6 = 2.0 * gama * nu * Pol[x] * Pol[y] * Pol[y] * Pol[y] / (r);
              Particles.ghost_get<11, 12, 13, 14, 15, 16>(SKIP_LABELLING);
              Df1[x] = Dx(f1);
              Df2[x] = Dx(f2);
              Df3[x] = Dx(f3);
              Df4[x] = Dx(f4);
              Df5[x] = Dx(f5);
              Df6[x] = Dx(f6);

              Df1[y] = Dy(f1);
              Df2[y] = Dy(f2);
              Df3[y] = Dy(f3);
              Df4[y] = Dy(f4);
              Df5[y] = Dy(f5);
              Df6[y] = Dy(f6);*/
            Particles.ghost_get<21, 22, 23, 24, 25, 26>(SKIP_LABELLING);


            dV[x] = -0.5 * Dy(h[y]) + zeta * Dx(delmu * Pol[x] * Pol[x]) + zeta * Dy(delmu * Pol[x] * Pol[y]) -
                    zeta * Dx(0.5 * delmu * (Pol[x] * Pol[x] + Pol[y] * Pol[y])) -
                    0.5 * nu * Dx(-2.0 * h[y] * Pol[x] * Pol[y])
                    - 0.5 * nu * Dy(h[y] * (Pol[x] * Pol[x] - Pol[y] * Pol[y])) - Dx(sigma[x][x]) -
                    Dy(sigma[x][y]) -
                    g[x]
                    - 0.5 * nu * Dx(-gama * lambda * delmu * (Pol[x] * Pol[x] - Pol[y] * Pol[y]))
                    - 0.5 * Dy(-2.0 * gama * lambda * delmu * (Pol[x] * Pol[y]));


            dV[y] = -0.5 * Dx(-h[y]) + zeta * Dy(delmu * Pol[y] * Pol[y]) + zeta * Dx(delmu * Pol[x] * Pol[y]) -
                    zeta * Dy(0.5 * delmu * (Pol[x] * Pol[x] + Pol[y] * Pol[y])) -
                    0.5 * nu * Dy(2.0 * h[y] * Pol[x] * Pol[y])
                    - 0.5 * nu * Dx(h[y] * (Pol[x] * Pol[x] - Pol[y] * Pol[y])) - Dx(sigma[y][x]) -
                    Dy(sigma[y][y]) -
                    g[y]
                    - 0.5 * nu * Dy(gama * lambda * delmu * (Pol[x] * Pol[x] - Pol[y] * Pol[y]))
                    - 0.5 * Dx(-2.0 * gama * lambda * delmu * (Pol[x] * Pol[y]));
            Particles.ghost_get<9>(SKIP_LABELLING);


            //Particles.write("PolarI");
            //Velocity Solution n iterations


            auto Stokes1 = eta * (Dxx(V[x]) + Dyy(V[x]))
                           + 0.5 * nu * (Df1[x] * Dx(V[x]) + f1 * Dxx(V[x]))
                           + 0.5 * nu * (Df2[x] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f2 * 0.5 * (Dxx(V[y]) + Dyx(V[x])))
                           + 0.5 * nu * (Df3[x] * Dy(V[y]) + f3 * Dyx(V[y]))
                           + 0.5 * nu * (Df4[y] * Dx(V[x]) + f4 * Dxy(V[x]))
                           + 0.5 * nu * (Df5[y] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f5 * 0.5 * (Dxy(V[y]) + Dyy(V[x])))
                           + 0.5 * nu * (Df6[y] * Dy(V[y]) + f6 * Dyy(V[y]));
            auto Stokes2 = eta * (Dxx(V[y]) + Dyy(V[y]))
                           - 0.5 * nu * (Df1[y] * Dx(V[x]) + f1 * Dxy(V[x]))
                           - 0.5 * nu * (Df2[y] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f2 * 0.5 * (Dxy(V[y]) + Dyy(V[x])))
                           - 0.5 * nu * (Df3[y] * Dy(V[y]) + f3 * Dyy(V[y]))
                           + 0.5 * nu * (Df4[x] * Dx(V[x]) + f4 * Dxx(V[x]))
                           + 0.5 * nu * (Df5[x] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f5 * 0.5 * (Dxx(V[y]) + Dyx(V[x])))
                           + 0.5 * nu * (Df6[x] * Dy(V[y]) + f6 * Dyx(V[y]));
            auto Stokes3 = eta * (Dxx(V[y]) + Dyy(V[y]))
                           - 0.5 * nu * (Df1[y] * Dx(V[x]) + f1 * Dxy(V[x]))
                           - 0.5 * nu * (Df2[y] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f2 * 0.5 * (Dxy(V[y]) + Dyy(V[x])))
                           - 0.5 * nu * (Df3[y] * Dy(V[y]) + f3 * Dyy(V[y]))
                           + 0.5 * nu * (Df4[x] * Dx(V[x]) + f4 * Dxx(V[x]))
                           + 0.5 * nu * (Df5[x] * 0.5 * (Dx(V[y]) + Dy(V[x])) + f5 * 0.5 * (Dxx(V[y]) + Dyx(V[x])))
                           + 0.5 * nu * (Df6[x] * Dy(V[y]) + f6 * Dyx(V[y]));
            tt.stop();
            std::cout << "Init of Velocity took " << tt.getwct() << " seconds." << std::endl;
            tt.start();
            V_err = 1;
            n = 0;
            errctr = 0;
            if (Vreset == 1) {
                P_bulk = 0;
                P = 0;
                Vreset = 0;
            }
            P = 0;
            P_bulk = 0;

            while (V_err >= V_err_eps && n <= nmax) {
                RHS[x] = dV[x];
                RHS[y] = dV[y];
                Particles.ghost_get<10>(SKIP_LABELLING);
                DCPSE_scheme<equations3d3, decltype(Particles)> Solver(Particles);
//                Solver.reset(Particles);
                Solver.impose(Stokes1, bulk, RHS[0], vx);
                Solver.impose(Stokes2, bulk, RHS[1], vy);
                Solver.impose(Stokes3, bulk, RHS[2], vz);
                Solver.impose(V[x], Surface, 0, vx);
                Solver.impose(V[y], Surface, 0, vy);
                Solver.impose(V[z], Surface, 0, vx);
                Solver.solve_with_solver(solverPetsc, V[x], V[y], V[z]);
                //Solver.solve(V[x], V[y]);
                Particles.ghost_get<Velocity>(SKIP_LABELLING);
                div = -(Dx(V[x]) + Dy(V[y]));
                for (int i = 0; i < bulk.size(); i++) {
                    Particles_subset.getProp<0>(i) = Particles.template getProp<4>(bulk.template get<0>(i));
                }
                for (int j = 0; j < Surface.size(); j++) {
                    auto p = Surface.get<0>(j);
                    Particles.getProp<4>(p) = 0;
                }
                sum = 0;
                sum1 = 0;
                for (int j = 0; j < bulk.size(); j++) {
                    auto p = bulk.get<0>(j);
                    sum += (Particles.getProp<18>(p)[0] - Particles.getProp<1>(p)[0]) *
                           (Particles.getProp<18>(p)[0] - Particles.getProp<1>(p)[0]) +
                           (Particles.getProp<18>(p)[1] - Particles.getProp<1>(p)[1]) *
                           (Particles.getProp<18>(p)[1] - Particles.getProp<1>(p)[1]);
                    sum1 += Particles.getProp<1>(p)[0] * Particles.getProp<1>(p)[0] +
                            Particles.getProp<1>(p)[1] * Particles.getProp<1>(p)[1];
                }
                sum = sqrt(sum);
                sum1 = sqrt(sum1);

                v_cl.sum(sum);
                v_cl.sum(sum1);
                v_cl.execute();
                V_t = V;
                Particles.ghost_get<1, 4, 18>(SKIP_LABELLING);
                V_err_old = V_err;
                V_err = sum / sum1;
                if (V_err > V_err_old || abs(V_err_old - V_err) < 1e-8) {
                    errctr++;
                    //alpha_P -= 0.1;
                } else {
                    errctr = 0;
                }
                if (n > 3) {
                    if (errctr > 3) {
                        std::cout << "CONVERGENCE LOOP BROKEN DUE TO INCREASE/VERY SLOW DECREASE IN ERROR" << std::endl;
                        Vreset = 1;
                        break;
                    } else {
                        Vreset = 0;
                    }
                }
                n++;
                //Particles.write_frame("V_debug", n);
                if (v_cl.rank() == 0) {
                    std::cout << "Rel l2 cgs err in V = " << V_err << " at " << n << std::endl;
                }
            }
            tt.stop();
            u[x][x] = Dx(V[x]);
            u[x][y] = 0.5 * (Dx(V[y]) + Dy(V[x]));
            u[y][x] = 0.5 * (Dy(V[x]) + Dx(V[y]));
            u[y][y] = Dy(V[y]);


            //Adaptive CFL
            /*sum=0;
            auto it2 = Particles.getDomainIterator();
            while (it2.isNext()) {
                auto p = it2.get();
                sum += Particles.getProp<Strain_rate>(p)[x][x] * Particles.getProp<Strain_rate>(p)[x][x] +
                        Particles.getProp<Strain_rate>(p)[y][y] * Particles.getProp<Strain_rate>(p)[y][y];
                ++it2;
            }
            sum = sqrt(sum);
            v_cl.sum(sum);
            v_cl.execute();
            dt=0.5/sum;*/
            if (v_cl.rank() == 0) {
                std::cout << "Rel l2 cgs err in V = " << V_err << " and took " << tt.getwct() << " seconds with " << n
                          << " iterations. dt is set to " << dt
                          << std::endl;
            }

            W[x][x] = 0;
            W[x][y] = 0.5 * (Dy(V[x]) - Dx(V[y]));
            W[y][x] = 0.5 * (Dx(V[y]) - Dy(V[x]));
            W[y][y] = 0;

            H_p_b = Pol[x] * Pol[x] + Pol[y] * Pol[y];
            for (int j = 0; j < bulk.size(); j++) {
                auto p = bulk.get<0>(j);
                Particles.getProp<32>(p) = (Particles.getProp<32>(p) == 0) ? 1 : Particles.getProp<32>(p);
            }
            for (int j = 0; j < Surface.size(); j++) {
                auto p = Surface.get<0>(j);
                Particles.getProp<32>(p) = (Particles.getProp<32>(p) == 0) ? 1 : Particles.getProp<32>(p);
            }

            h[x] = -gama * (lambda * delmu - nu * (u[x][x] * Pol[x] * Pol[x] + u[y][y] * Pol[y] * Pol[y] +
                                                   2 * u[x][y] * Pol[x] * Pol[y]) / (H_p_b));


            //Particles.ghost_get<MolField, Strain_rate, Vorticity>(SKIP_LABELLING);
            //Particles.write_frame("Polar_withGhost_3e-3", ctr);
            /*Particles.deleteGhost();
            Particles.write_frame("Polar", ctr);
            Particles.ghost_get<0>();*/

            ctr++;

            H_p_b = sqrt(H_p_b);

            k1[x] = ((h[x] * Pol[x] - h[y] * Pol[y]) / gama + lambda * delmu * Pol[x] -
                     nu * (u[x][x] * Pol[x] + u[x][y] * Pol[y]) + W[x][x] * Pol[x] +
                     W[x][y] * Pol[y]);// - V[x] * Dx(Pol[x]) - V[y] * Dy(Pol[x]));
            k1[y] = ((h[x] * Pol[y] + h[y] * Pol[x]) / gama + lambda * delmu * Pol[y] -
                     nu * (u[y][x] * Pol[x] + u[y][y] * Pol[y]) + W[y][x] * Pol[x] +
                     W[y][y] * Pol[y]);// - V[x] * Dx(Pol[y]) - V[y] * Dy(Pol[y]));

            H_t = H_p_b;//+0.5*dt*(k1[x]*k1[x]+k1[y]*k1[y]);
            dPol = Pol + (0.5 * dt) * k1;
            dPol = dPol / H_t;
            r = dPol[x] * dPol[x] + dPol[y] * dPol[y];
            for (int j = 0; j < bulk.size(); j++) {
                auto p = bulk.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
            }
            for (int j = 0; j < Surface.size(); j++) {
                auto p = Surface.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
                Particles.getProp<8>(p)[x] = sin(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
                Particles.getProp<0>(p)[y] = cos(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
            }
            Particles.ghost_get<8>(SKIP_LABELLING);


            h[y] = (dPol[x] * (Ks * Dyy(dPol[y]) + Kb * Dxx(dPol[y]) + (Ks - Kb) * Dxy(dPol[x])) -
                    dPol[y] * (Ks * Dxx(dPol[x]) + Kb * Dyy(dPol[x]) + (Ks - Kb) * Dxy(dPol[y])));

            h[x] = -gama * (lambda * delmu - nu * ((u[x][x] * dPol[x] * dPol[x] + u[y][y] * dPol[y] * dPol[y] +
                                                    2 * u[x][y] * dPol[x] * dPol[y]) / (r)));

            k2[x] = ((h[x] * (dPol[x]) - h[y] * (dPol[y])) / gama +
                     lambda * delmu * (dPol[x]) -
                     nu * (u[x][x] * (dPol[x]) + u[x][y] * (dPol[y])) +
                     W[x][x] * (dPol[x]) + W[x][y] * (dPol[y])); //-V[x] * Dx((dPol[x])) - V[y] * Dy((dPol[x])));
            k2[y] = ((h[x] * (dPol[y]) + h[y] * (dPol[x])) / gama +
                     lambda * delmu * (dPol[y]) -
                     nu * (u[y][x] * (dPol[x]) + u[y][y] * (dPol[y])) +
                     W[y][x] * (dPol[x]) + W[y][y] * (dPol[y])); //-V[x] * Dx((dPol[y])) - V[y] * Dy((dPol[y])));

            H_t = H_p_b;//+0.5*dt*(k2[x]*k2[x]+k2[y]*k2[y]);
            dPol = Pol + (0.5 * dt) * k2;
            dPol = dPol / H_t;
            r = dPol[x] * dPol[x] + dPol[y] * dPol[y];
            for (int j = 0; j < bulk.size(); j++) {
                auto p = bulk.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
            }
            for (int j = 0; j < Surface.size(); j++) {
                auto p = Surface.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
                Particles.getProp<8>(p)[x] = sin(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
                Particles.getProp<0>(p)[y] = cos(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
            }
            Particles.ghost_get<8>(SKIP_LABELLING);

            h[y] = (dPol[x] * (Ks * Dyy(dPol[y]) + Kb * Dxx(dPol[y]) + (Ks - Kb) * Dxy(dPol[x])) -
                    dPol[y] * (Ks * Dxx(dPol[x]) + Kb * Dyy(dPol[x]) + (Ks - Kb) * Dxy(dPol[y])));

            h[x] = -gama * (lambda * delmu - nu * ((u[x][x] * dPol[x] * dPol[x] + u[y][y] * dPol[y] * dPol[y] +
                                                    2 * u[x][y] * dPol[x] * dPol[y]) / (r)));

            k3[x] = ((h[x] * (dPol[x]) - h[y] * (dPol[y])) / gama +
                     lambda * delmu * (dPol[x]) -
                     nu * (u[x][x] * (dPol[x]) + u[x][y] * (dPol[y])) +
                     W[x][x] * (dPol[x]) + W[x][y] * (dPol[y]));
            // -V[x] * Dx((dPol[x])) - V[y] * Dy((dPol[x])));
            k3[y] = ((h[x] * (dPol[y]) + h[y] * (dPol[x])) / gama +
                     lambda * delmu * (dPol[y]) -
                     nu * (u[y][x] * (dPol[x]) + u[y][y] * (dPol[y])) +
                     W[y][x] * (dPol[x]) + W[y][y] * (dPol[y]));
            // -V[x] * Dx((dPol[y])) - V[y] * Dy((dPol[y])));
            H_t = H_p_b;//+dt*(k3[x]*k3[x]+k3[y]*k3[y]);
            dPol = Pol + (dt * k3);
            dPol = dPol / H_t;
            Particles.ghost_get<8>(SKIP_LABELLING);
            r = dPol[x] * dPol[x] + dPol[y] * dPol[y];
            for (int j = 0; j < bulk.size(); j++) {
                auto p = bulk.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
            }
            for (int j = 0; j < Surface.size(); j++) {
                auto p = Surface.get<0>(j);
                Particles.getProp<34>(p) = (Particles.getProp<34>(p) == 0) ? 1 : Particles.getProp<34>(p);
                Particles.getProp<8>(p)[x] = sin(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
                Particles.getProp<0>(p)[y] = cos(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
            }
            Particles.ghost_get<8>(SKIP_LABELLING);

            h[y] = (dPol[x] * (Ks * Dyy(dPol[y]) + Kb * Dxx(dPol[y]) + (Ks - Kb) * Dxy(dPol[x])) -
                    dPol[y] * (Ks * Dxx(dPol[x]) + Kb * Dyy(dPol[x]) + (Ks - Kb) * Dxy(dPol[y])));

            h[x] = -gama * (lambda * delmu - nu * ((u[x][x] * dPol[x] * dPol[x] + u[y][y] * dPol[y] * dPol[y] +
                                                    2 * u[x][y] * dPol[x] * dPol[y]) / (r)));

            k4[x] = ((h[x] * (dPol[x]) - h[y] * (dPol[y])) / gama +
                     lambda * delmu * (dPol[x]) -
                     nu * (u[x][x] * (dPol[x]) + u[x][y] * (dPol[y])) +
                     W[x][x] * (dPol[x]) +
                     W[x][y] * (dPol[y]));//   -V[x]*Dx( (dt * k3[x]+Pol[x])) -V[y]*Dy( (dt * k3[x]+Pol[x])));
            k4[y] = ((h[x] * (dPol[y]) + h[y] * (dPol[x])) / gama +
                     lambda * delmu * (dPol[y]) -
                     nu * (u[y][x] * (dPol[x]) + u[y][y] * (dPol[y])) +
                     W[y][x] * (dPol[x]) +
                     W[y][y] * (dPol[y]));//  -V[x]*Dx( (dt * k3[y]+Pol*[y])) -V[y]*Dy( (dt * k3[y]+Pol[y])));

            Pol = Pol + (dt / 6.0) * (k1 + (2.0 * k2) + (2.0 * k3) + k4);
            Pol = Pol / H_p_b;

            H_p_b = sqrt(Pol[x] * Pol[x] + Pol[y] * Pol[y]);
            Pol = Pol / H_p_b;
            for (int j = 0; j < Surface.size(); j++) {
                auto p = Surface.get<0>(j);
                Particles.getProp<0>(p)[x] = sin(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
                Particles.getProp<0>(p)[y] = cos(2 * M_PI * (cos((2 * Particles.getPos(p)[x] - Lx) / Lx) -
                                                             sin((2 * Particles.getPos(p)[y] - Ly) / Ly)));
            }
            k1 = V;
            k2 = 0.5 * dt * k1 + V;
            k3 = 0.5 * dt * k2 + V;
            k4 = dt * k3 + V;
            //Pos = Pos + dt * V;
            Pos = Pos + dt / 6.0 * (k1 + 2 * k2 + 2 * k3 + k4);

            Particles.map();

            Particles.ghost_get<0, ExtForce, 27>();
            //indexUpdate(Particles, Particles_subset, up_p, dw_p, l_p, r_p, up_p1, dw_p1, l_p1, r_p1, corner_ul,corner_ur, corner_dl, corner_dr, bulk, up, down, left, right);
            //Particles_subset.write("debug");

            tt.start();
            Dx.update(Particles);
            Dy.update(Particles);
            Dxy.update(Particles);
            auto Dyx = Dxy;
            Dxx.update(Particles);
            Dyy.update(Particles);

            Bulk_Dx.update(Particles_subset);
            Bulk_Dy.update(Particles_subset);

            tt.stop();
            if (v_cl.rank() == 0) {
                std::cout << "Updation of operators took " << tt.getwct() << " seconds." << std::endl;
                std::cout << "Time step " << ctr - 1 << " : " << tim << " over." << std::endl;
                std::cout << "----------------------------------------------------------" << std::endl;
            }

            tim += dt;
        }

        Particles.deleteGhost();
        Particles.write("Polar_Last");

        Dx.deallocate(Particles);
        Dy.deallocate(Particles);
        Dxy.deallocate(Particles);
        Dxx.deallocate(Particles);
        Dyy.deallocate(Particles);
        Bulk_Dx.deallocate(Particles_subset);
        Bulk_Dy.deallocate(Particles_subset);
        Particles.deleteGhost();
        tt2.stop();
        if (v_cl.rank() == 0) {
            std::cout << "The simulation took " << tt2.getcputime() << "(CPU) ------ " << tt2.getwct()
                      << "(Wall) Seconds.";
        }
    }


BOOST_AUTO_TEST_SUITE_END()