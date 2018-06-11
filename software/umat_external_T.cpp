/* This file is part of simcoon.
 
 simcoon is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 simcoon is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with simcoon.  If not, see <http://www.gnu.org/licenses/>.
 
 */

///@file umat_singleM.cpp
///@brief umat template to run smart subroutines using Abaqus
///@brief Implemented in 1D-2D-3D
///@author Chemisky & Despringre
///@version 1.0
///@date 12/04/2013

#include <iostream>
#include <fstream>
#include <assert.h>
#include <string.h>
#include <armadillo>

#include <simcoon/parameter.hpp>
#include <simcoon/Simulation/Phase/phase_characteristics.hpp>
#include <simcoon/Simulation/Phase/state_variables_T.hpp>
#include <simcoon/Continuum_mechanics/Umat/umat_smart.hpp>

#include <simcoon/Simulation/Maths/lagrange.hpp>
#include <simcoon/Simulation/Maths/rotation.hpp>
#include <simcoon/Simulation/Maths/num_solve.hpp>
#include <simcoon/Continuum_mechanics/Functions/contimech.hpp>
#include <simcoon/Continuum_mechanics/Functions/constitutive.hpp>
#include <simcoon/Continuum_mechanics/Functions/recovery_props.hpp>
#include <simcoon/Continuum_mechanics/Functions/criteria.hpp>

///@param stress array containing the components of the stress tensor (dimension ntens)
///@param statev array containing the evolution variables (dimension nstatev)
///@param ddsdde array containing the mechanical tangent operator (dimension ntens*ntens)
///@param sse unused
///@param spd unused
///@param scd unused
///@param rpl unused
///@param ddsddt array containing the thermal tangent operator
///@param drple unused
///@param drpldt unused
///@param stran array containing total strain component (dimension ntens) at the beginning of increment
///@param dstran array containing the component of total strain increment (dimension ntens)
///@param time two compoenent array : first component is the value of step time at the beginning of the current increment and second component is the value of total time at the beginning of the current increment
///@param dtime time increment
///@param temperature temperature avlue at the beginning of increment
///@param Dtemperature temperature increment
///@param predef unused
///@param dpred unused
///@param cmname user-defined material name
///@param ndi number of direct stress components
///@param nshr number of shear stress components
///@param ntens number stress and strain components
///@param nstatev number of evolution variables
///@param props array containing material properties
///@param nprops number of material properties
///@param coords coordinates of the considered point
///@param drot rotation increment matrix (dimension 3*3)
///@param pnewdt ratio of suggested new time increment
///@param celent characteristic element length
///@param dfgrd0 array containing the deformation gradient at the beginning of increment (dimension 3*3)
///@param dfgrd1 array containing the deformation gradient at the end of increment (dimension 3*3)
///@param noel element number
///@param npt integration point number
///@param layer layer number - not used
///@param kspt section point number within the current layer - not used
///@param kstep step number
///@param kinc increment number

using namespace std;
using namespace arma;
using namespace simcoon;

void umat_sma_unified_T_T(const arma::vec &, const arma::vec &, arma::vec &, double &, arma::mat &, arma::mat &, arma::mat &, arma::mat &, const arma::mat &, const int &, const arma::vec &, const int &, arma::vec &, const double &, const double &,const double &,const double &, double &, double &, double &, double &, double &, double &, double &, const int &, const int &, const bool &, double &);

extern "C" void umat_(double *stress, double *statev, double *ddsdde, double &sse, double &spd, double &scd, double &rpl, double *ddsddt, double *drplde, double &drpldt, const double *stran, const double *dstran, const double *time, const double &dtime, const double &temperature, const double &Dtemperature, const double &predef, const double &dpred, char *cmname, const int &ndi, const int &nshr, const int &ntens, const int &nstatev, const double *props, const int &nprops, const double &coords, const double *drot, double &pnewdt, const double &celent, const double *dfgrd0, const double *dfgrd1, const int &noel, const int &npt, const double &layer, const int &kspt, const int &kstep, const int &kinc)
{
    UNUSED(sse);
    UNUSED(spd);
	UNUSED(scd);
	UNUSED(rpl);
	UNUSED(ddsddt);
	UNUSED(drplde);
	UNUSED(drpldt);
	UNUSED(predef);
	UNUSED(dpred);
	UNUSED(ntens);
	UNUSED(coords);
	UNUSED(celent);
	UNUSED(dfgrd0);
	UNUSED(dfgrd1);
	UNUSED(noel);
	UNUSED(npt);
	UNUSED(layer);
	UNUSED(kspt);
	UNUSED(kstep);
	UNUSED(kinc);
	
	bool start = false;
	double Time = 0.;
	double DTime = 0.;
    int solver_type = 0;
    
	int nstatev_smart = nstatev-7;
    vec props_smart = zeros(nprops);
    vec statev_smart = zeros(nstatev_smart);
    
    vec sigma = zeros(6);
    vec Etot = zeros(6);
    vec DEtot = zeros(6);    
    mat dSdE = zeros(6,6);
    mat dSdT = zeros(6,1);
    mat drpldE = zeros(6,1);
    mat drpldT = zeros(1,1);
    mat DR = zeros(3,3);
    vec Wm = zeros(4);
    vec Wt = zeros(3);
    double T = 0.;
    double DT = 0.;
    double r = 0.;
    
	abaqus2smart_T(stress, ddsdde, ddsddt, drplde, drpldt, stran, dstran, time, dtime, temperature, Dtemperature, nprops, props, nstatev, statev, ndi, nshr, drot, sigma, dSdE, dSdT, drpldE, drpldT, Etot, DEtot, T, DT, Time, DTime, props_smart, Wm, Wt, statev_smart, DR, start);
    umat_sma_unified_T_T(Etot, DEtot, sigma, r, dSdE, dSdT, drpldE, drpldT, DR, nprops, props_smart, nstatev_smart, statev_smart, T, DT, Time, DTime, Wm(0), Wm(1), Wm(2), Wm(3),  Wt(0), Wt(1), Wt(2), ndi, nshr, start, pnewdt);
	smart2abaqus_T(stress, ddsdde, ddsddt, drplde, drpldt, rpl, statev, ndi, nshr, sigma, statev_smart, r, Wm, Wt, dSdE, dSdT, drpldE, drpldT);
}

void umat_sma_unified_T_T(const vec &Etot, const vec &DEtot, vec &sigma, double &r, mat &dSdE, mat &dSdT, mat &drdE, mat &drdT, const mat &DR, const int &nprops, const vec &props, const int &nstatev, vec &statev, const double &T, const double &DT,const double &Time,const double &DTime, double &Wm, double &Wm_r, double &Wm_ir, double &Wm_d, double &Wt, double &Wt_r, double &Wt_ir, const int &ndi, const int &nshr, const bool &start, double &tnew_dt) {
    
    UNUSED(nprops);
    UNUSED(nstatev);
    UNUSED(Time);
    UNUSED(nshr);
    UNUSED(tnew_dt);
    
    ///@brief The thermomechanical transformation UMAT for SMAs has the following statev and props

    ///@brief props[0] : density
    ///@brief props[1] : specific heat capacity
    ///@brief props[2] : flagT: 0 transformation temperatures linearly extrapolated; 1 : smooth temperatures
    ///@brief props[3] : EA: Young's modulus of Austenite
    ///@brief props[4] : EM: Young's modulus of Martensite
    ///@brief props[5] : nuA : Poisson's ratio of Austenite
    ///@brief props[6] : nuM : Poisson's ratio of Martensite
    ///@brief props[7] : alphaA_iso : CTE of Austenite
    ///@brief props[8] : alphaM_iso : CTE of Martensite
    ///@brief props[9] : Hmin : Minimal transformation strain magnitude
    ///@brief props[10] : Hmax : Maximal transformation strain magnitude
    ///@brief props[11] : k1 : Exponential evolution of transformation strain magnitude
    ///@brief props[12] : sigmacrit : Critical stress for change of transformation strain magnitude
    ///@brief props[13]: C_A : Slope of martesnite -> austenite parameter
    ///@brief props[14]: C_M : Slope of austenite -> martensite parameter
    ///@brief props[15]: Ms0 : Martensite start at zero stress
    ///@brief props[16]: Mf0 : Martensite finish at zero stress
    ///@brief props[17]: As0 : Austenite start at zero stress
    ///@brief props[18]: Af0 : Austenite finish at zero stress
    ///@brief props[19]: n1 : Martensite start smooth exponent
    ///@brief props[20]: n2 : Martensite finish smooth exponent
    ///@brief props[21]: n3 : Austenite start smooth exponent
    ///@brief props[22]: n4 : Austenite finish smooth exponent
    ///@brief props[23]: sigmacaliber : Stress at which the slopes CA and CM are identified
    ///@brief props[24]: prager_b : Tension-compression asymmetry parameter
    ///@brief props[25]: prager_n : Tension-compression asymmetry exponent
    ///@brief props[26]: c_lambda : penalty function exponent start point
    ///@brief props[27]: p0_lambda : penalty function exponent limit penalty value
    ///@brief props[28]: n_lambda : penalty function power law exponent
    ///@brief props[29]: alpha_lambda : penalty function power law parameter
    
    ///@brief The elastic-plastic UMAT with isotropic hardening requires 14 statev:
    ///@brief statev[0] : T_init : Initial temperature
    ///@brief statev[1] : xi : MVF (Martensitic volume fraction)
    ///@brief statev[2] : Transformation strain 11: ET(0,0)
    ///@brief statev[3] : Transformation strain 22: ET(1,1)
    ///@brief statev[4] : Transformation strain 33: ET(2,2)
    ///@brief statev[5] : Transformation strain 12: ET(0,1) (*2)
    ///@brief statev[6] : Transformation strain 13: ET(0,2) (*2)
    ///@brief statev[7] : Transformation strain 23: ET(1,2) (*2)
    ///@brief statev[8] : xiF : forward MVF
    ///@brief statev[9] : xiR : reverse MVF
    ///@brief statev[10] : rhoDs0 : difference in entropy for the phases (M - A)
    ///@brief statev[11] : rhoDE0 : difference in internal energy for the phases (M - A)
    ///@brief statev[12] : parameter for the stress dependance of transformation limits
    ///@brief statev[13] : a1 : forward hardening parameter
    ///@brief statev[14] : a2 : reverse hardening parameter
    ///@brief statev[15] : a3 : Equilibrium hardening parameter
    ///@brief statev[16] : Y0t : Initial transformation critical value
    
    // Taking the values from the material created
    double rho = props(0);
    double c_pA = props(1);
    double c_pM = props(2);
    int flagT = props(3);
    double E_A = props(4);
    double E_M = props(5);
    double nu_A = props(6);
    double nu_M= props(7);
    double alphaA_iso = props(8);
    double alphaM_iso = props(9);
    //parameters for Hcur
    double Hmin = props(10);
    double Hmax = props(11);
    double k1 = props(12);
    double sigmacrit = props(13);
    //parameters for phase transformation
    double C_A = props(14);
    double C_M = props(15);
    double Ms0 = props(16);
    double Mf0 = props(17);
    double As0 = props(18);
    double Af0 = props(19);
    //Additional parameters
    double n1 = props(20);
    double n2 = props(21);
    double n3 = props(22);
    double n4 = props(23);
    double sigmacaliber = props(24);
    double prager_b = props(25);
    double prager_n = props(26);
    double sigmastar = 0.;
    
    //Set the Lagrange multipliers coefficients
    double c_lambda = props(27);
    double p0_lambda = props(28);
    double n_lambda = props(29);
    double alpha_lambda = props(30);
    
    ///@brief Temperature initialization
    double T_init = statev(0);
    ///@brief Martensite volume fraction initialization
    double xi = statev(1);
    ///@brief mean strain tensor creation
    vec ET = zeros(6);
    ET(0) = statev(2);
    ET(1) = statev(3);
    ET(2) = statev(4);
    ET(3) = statev(5);
    ET(4) = statev(6);
    ET(5) = statev(7);
    
    ///@brief ETMax allow the definition of the lambdaTR
    vec DETF = zeros(6);
    vec DETR = zeros(6);
    vec ETMean = zeros(6);
    
    double xiF = statev(8);
    double xiR = statev(9);
    
    double rhoDs0 = statev(10);
    double rhoDE0 = statev(11);
    double D = statev(12);
    double a1 = statev(13);
    double a2 = statev(14);
    double a3 = statev(15);
    double Y0t = statev(16);
    
    //Elastic stiffness tensor and thermal tensor
    //defines K and mu explicitely
    //Find the elastic stiffness tensor that is dependent on fraction volume of martensite
    double K_A = E_A/(3.*(1.-2*nu_A));
    double mu_A = E_A/(2.*(1.+nu_A));
    
    double K_M = E_M/(3.*(1.-2*nu_M));
    double mu_M = E_M/(2.*(1.+nu_M));
    
    double K_eff = (K_A*K_M)/(xi*K_A + (1. - xi)*K_M);
    double mu_eff = (mu_A*mu_M)/(xi*mu_A + (1. - xi)*mu_M);
    
    //defines M_A and M_M
    mat M_A = M_iso(K_A, mu_A, "Kmu");
    mat M_M = M_iso(K_M, mu_M, "Kmu");
    mat M = M_iso(K_eff, mu_eff, "Kmu");
    mat L = L_iso(K_eff, mu_eff, "Kmu");
    mat DM = M_M - M_A;
    
    //definition of the CTE tensor
    vec alpha = (alphaM_iso*xi + alphaA_iso*(1.-xi))*Ith();
    vec Dalpha = (alphaM_iso - alphaA_iso)*Ith();
    
    ///@brief Initialization
    if(start) {
        
        T_init = T;
        vec vide = zeros(6);
        sigma = zeros(6);
        ET = zeros(6);
        xiF = limit;
        xiR = 0.;
        xi = xiF;
        
        Wm = 0.;
        Wm_r = 0.;
        Wm_ir = 0.;
        Wm_d = 0.;
        
        //Get Hcurstar and its derivative to determine rhoDs0
        if (sigmacaliber > sigmacrit)
            sigmastar = sigmacaliber - sigmacrit;
        else
            sigmastar = 0.;
        
        double Hcurstar = Hmin + (Hmax - Hmin)*(1. - exp(-k1*sigmastar));
        assert(Hcurstar > 1E-12);
        
        double dHcurstar = (Hmax - Hmin)*k1*exp(-k1*sigmastar);
        rhoDs0 = -2.*C_M*C_A*(Hcurstar + sigmacaliber*(dHcurstar + (1/E_M-1/E_A)))/(C_M+C_A);
        
        //Determination of the Smooth temperatures
        double MsSmooth = 0.;
        double MfSmooth = 0.;
        double AsSmooth = 0.;
        double AfSmooth = 0.;
        
        if(flagT == 0) {
            MsSmooth = 0.5*Ms0*(1.+ (n1+1)*pow(2.,-n1) + (n2-1)*pow(2.,-n2))/(n1*pow(2.,-n1) + n2*pow(2.,-n2)) + 0.5*Mf0*(-1.+ (n1-1)*pow(2.,-n1) + (n2+1)*pow(2.,-n2))/(n1*pow(2.,-n1) + n2*pow(2.,-n2));
            MfSmooth = 0.5*Ms0*(-1.+ (n1+1)*pow(2.,-n1) + (n2-1)*pow(2.,-n2))/(n1*pow(2.,-n1) + n2*pow(2.,-n2)) + 0.5*Mf0*(1.+ (n1-1)*pow(2.,-n1) + (n2+1)*pow(2.,-n2))/(n1*pow(2.,-n1) + n2*pow(2.,-n2));
            AsSmooth = 0.5*As0*(1.+ (n3-1)*pow(2.,-n3) + (n4+1)*pow(2.,-n4))/(n3*pow(2.,-n3) + n4*pow(2.,-n4)) + 0.5*Af0*(-1.+ (n3+1)*pow(2.,-n3) + (n4-1)*pow(2.,-n4))/(n3*pow(2.,-n3) + n4*pow(2.,-n4));
            AfSmooth = 0.5*As0*(-1.+ (n3-1)*pow(2.,-n3) + (n4+1)*pow(2.,-n4))/(n3*pow(2.,-n3) + n4*pow(2.,-n4)) + 0.5*Af0*(1.+ (n3+1)*pow(2.,-n3) + (n4-1)*pow(2.,-n4))/(n3*pow(2.,-n3) + n4*pow(2.,-n4));
        }
        else {
            MsSmooth = Ms0;
            MfSmooth = Mf0;
            AsSmooth = As0;
            AfSmooth = Af0;
        }
        
        rhoDE0 = 0.5*rhoDs0*(MsSmooth + AfSmooth);
        D = (C_M-C_A)*(Hcurstar + sigmacaliber*(dHcurstar + (1/E_M-1/E_A)))/((C_A+C_M)*(Hcurstar + sigmacaliber*dHcurstar));
        a1 = rhoDs0*(MfSmooth - MsSmooth);
        a2 = rhoDs0*(AsSmooth - AfSmooth);
        a3 = -0.25*a1*(1 + 1./(n1 + 1.) - 1./(n2 + 1.))+0.25*a2*(1. + 1./(n3 + 1.) - 1./(n4 + 1.));
        Y0t = 0.5*rhoDs0*(MsSmooth - AfSmooth) - a3;
    }
    
    //Rotation of internal variables (tensors)
    rotate_strain(ET, DR);
    
    //Variables values at the start of the increment
    vec	sigma_start = sigma;
    vec ET_start = ET;
    double xi_start = xi;
    vec alpha_start = alpha;
    
    // Find Hcur explicit
    if (Mises_stress(sigma) > sigmacrit)
        sigmastar = Mises_stress(sigma) - sigmacrit;
    else
        sigmastar = 0.;
    
    double Hcur = Hmin + (Hmax - Hmin)*(1. - exp(-1.*k1*sigmastar));
    
    //definition of Lambdas associated to transformation
    vec lambdaTF = Hcur*dPrager_stress(sigma, prager_b, prager_n);
    
    if (Mises_strain(ET) > 1E-6)
        ETMean = dev(ET) / (xi);
    else if (Mises_stress(sigma) < 1.E-6)
        ETMean = lambdaTF;
    else
        ETMean = 0.*Ith();
    
    vec lambdaTR = -1.*ETMean;
    
    //Definition of the modified Y function
    double YtF = Y0t + D*Hcur*Mises_stress(sigma);
    double YtR = Y0t + D*sum(sigma%ETMean); //*D Changed by D.
    
    double HfF = 0.;
    double HfR = 0.;
    
    //Hardening function definition (Smooth hardening functions)
    if ((xi > 0.)&&((1. - xi) > 0.)) {
        HfF = 0.5*a1*(1. + pow(xi,n1) - pow(1. - xi,n2)) + a3;
        HfR = 0.5*a2*(1. + pow(xi,n3) - pow((1. - xi),n4)) - a3;
    }
    else if ((xi <= 0.)&&((1. - xi) > 0.)) {
        HfF = 0.5*a1*(1. - pow(1. - xi,n2)) + a3;
        HfR = 0.5*a2*(1. - pow((1. - xi),n4)) - a3;
    }
    else if ((xi > 0.)&&((1. - xi) <= 0.)) {
        HfF = 0.5*a1*(1. + pow(xi,n1)) + a3;
        HfR = 0.5*a2*(1. + pow(xi,n3)) - a3;
    }
    else {
        HfF =  a3;
        HfR = - a3;
    }
    
    //Set the Lagrange multiplliers due to Physical limitations
    double lambda0 = -1.*lagrange_pow_0(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
    double lambda1 = lagrange_pow_1(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
    
    //Define the value of DM_sig
    vec DM_sig = (DM*sigma_start);
    //Define the value of Dalpha_T
    vec Dalpha_T = Dalpha*(T+DT);
    
    //Set the thermo forces
    double A_xiF = rhoDs0*(T+DT) - rhoDE0 + 0.5*sum(sigma%DM_sig) + sum(sigma%Dalpha)*(T+DT-T_init) - HfF;
    double A_xiF_start = rhoDs0*(T) - rhoDE0 + 0.5*sum(sigma_start%DM_sig) + sum(sigma_start%Dalpha)*(T-T_init) - HfF;
    double A_xiR = -1.*rhoDs0*(T+DT) + rhoDE0 - 0.5*sum(sigma%DM_sig) - sum(sigma%Dalpha)*(T+DT-T_init) + HfR;
    double A_xiR_start = -1.*rhoDs0*(T) + rhoDE0 - 0.5*sum(sigma_start%DM_sig) - sum(sigma_start%Dalpha)*(T-T_init) + HfR;
    
    //Transformation criteria
    double PhihatF = Hcur*Prager_stress(sigma, prager_b, prager_n);
    double PhihatR = sum(sigma%ETMean);
    
    //Variables required for the loop
    vec s_j = zeros(2);
    s_j(0) = xiF;
    s_j(1) = xiR;
    vec Ds_j = zeros(2);
    vec ds_j = zeros(2);
    
    ///Elastic prediction - Accounting for the thermal prediction
    vec Eel = Etot + DEtot - alpha*(T+DT-T_init) - ET;
    sigma = el_pred(L, Eel, ndi);
    
    //Define the functions for the system to solve
    vec Phi = zeros(2);
    mat B = zeros(2,2);
    vec Y_crit = zeros(2);
    
    //Define the function for the system to solve
    double dHfF = 0.;
    double dHfR = 0.;
    vec dHcurdsigma = zeros(6);
    //Relative to forward transformation
    vec dPhihatFdsigma = zeros(6);
    double dPhihatFdxiF = 0.;
    double dPhihatFdxiR = 0.;
    
    vec dA_xiFdsigma = zeros(6);
    double dA_xiFdxiF = 0.;
    double dA_xiFdxiR = 0.;
    
    vec dlambda1dsigma = zeros(6);
    double dlambda1dxiF = 0.;
    double dlambda1dxiR = 0.;
    
    vec dYtFdsigma = zeros(6);
    double dYtFdxiF = 0.;
    double dYtFdxiR = 0.;
    
    vec dPhiFdsigma = zeros(6);
    double dPhiFdxiF = 0.;
    double dPhiFdxiR = 0.;
    
    //Relative to reverse transformation
    vec dPhihatRdsigma = zeros(6);
    double dPhihatRdxiF = 0.;
    double dPhihatRdxiR = 0.;
    vec dPhihatRdETF = zeros(6);
    vec dPhihatRdETR = zeros(6);
    
    vec dA_xiRdsigma = zeros(6);
    double dA_xiRdxiF = 0.;
    double dA_xiRdxiR = 0.;
    
    vec dlambda0dsigma = zeros(6);
    double dlambda0dxiF = 0.;
    double dlambda0dxiR = 0.;
    
    vec dYtRdsigma = zeros(6);
    double dYtRdxiF = 0.;
    double dYtRdxiR = 0.;
    vec dYtRdETF = zeros(6);
    vec dYtRdETR = zeros(6);
    
    vec dPhiRdsigma = zeros(6);
    double dPhiRdxiF = 0.;
    double dPhiRdxiR = 0.;
    vec dPhiRdETF = zeros(6);
    vec dPhiRdETR = zeros(6);
    
    //Compute the explicit flow direction
    std::vector<vec> kappa_j(2);
    mat K = zeros(2,2);
    
    //Loop parameters
    int compteur = 0;
    double error = 1.;
    
    //Loop
    for (compteur = 0; ((compteur < maxiter_umat) && (error > precision_umat)); compteur++) {
        
        K_eff = (K_A*K_M) / (xi*K_A + (1. - xi)*K_M);
        mu_eff = (mu_A*mu_M) / (xi*mu_A + (1. - xi)*mu_M);
        L = L_iso(K_eff, mu_eff, "Kmu");
        M = M_iso(K_eff, mu_eff, "Kmu");
        
        DM_sig = DM*sigma;
        Dalpha_T = Dalpha*(T+DT);
        
        lambdaTF = Hcur * dPrager_stress(sigma, prager_b, prager_n);
        lambdaTR = -1. * ETMean;
        
        kappa_j[0] = L*(lambdaTF + DM_sig + Dalpha_T);
        kappa_j[1] = L*(lambdaTR - DM_sig - Dalpha_T); //derivative w/ xiR is minus derivative w/ xi
        
        //Hardening function definition (Smooth hardening functions)
        if ((xi > 0.) && ((1. - xi) > 0.)) {
            HfF = 0.5*a1*(1. + pow(xi, n1) - pow(1. - xi, n2)) + a3;
        }
        else if ((xi <= 0.) && ((1. - xi) > 0.)) {
            HfF = 0.5*a1*(1. - pow(1. - xi, n2)) + a3;
        }
        else if ((xi > 0.) && ((1. - xi) <= 0.)) {
            HfF = 0.5*a1*(1. + pow(xi, n1)) + a3;
        }
        else  {
            HfF = 0.5*a1 + a3;
        }
        
        if ((xi > 0.) && ((1. - xi) > 0.)) {
            HfR = 0.5*a2*(1. + pow(xi, n3) - pow((1. - xi), n4)) - a3;
        }
        else if ((xi <= 0.) && ((1. - xi) > 0.)) {
            HfR = 0.5*a2*(1. - pow((1. - xi), n4)) - a3;
        }
        else if ((xi > 0.) && ((1. - xi) <= 0.)) {
            HfR = 0.5*a2*(1. + pow(xi, n3)) - a3;
        }
        else
            HfR = 0.5*a2* -a3;
        
        // Find Hcur explicit
        if (Mises_stress(sigma) > sigmacrit)
            sigmastar = Mises_stress(sigma) - sigmacrit;
        else
            sigmastar = 0.;
        
        Hcur = Hmin + (Hmax - Hmin)*(1. - exp(-1.*k1*sigmastar));
        
        //Forward transformation thermodynamic force
        PhihatF = Hcur*Prager_stress(sigma, prager_b, prager_n);
        A_xiF = rhoDs0*(T + DT) - rhoDE0 + 0.5*sum(sigma%DM_sig) + sum(sigma%Dalpha)*(T + DT - T_init) - HfF;
        lambda1 = lagrange_pow_1(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        YtF = Y0t + D*Hcur*Mises_stress(sigma);
        Phi(0) = PhihatF + A_xiF - lambda1 - YtF;
        
        //Reverse transformation thermodynamic force
        PhihatR = sum(sigma%ETMean);
        A_xiR = -1.*rhoDs0*(T + DT) + rhoDE0 - 0.5*sum(sigma%DM_sig) - sum(sigma%Dalpha)*(T + DT - T_init) + HfR;
        lambda0 = -1.*lagrange_pow_0(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        YtR = Y0t + D*sum(sigma%ETMean);
        Phi(1) = -1.*PhihatR + A_xiR + lambda0 - YtR;  // PhiR < 0.
        
        if ((xi > 0.) && ((1. - xi) > 0.)) {
            dHfF = 0.5*a1*(n1*pow(xi, n1 - 1.) + n2*pow(1. - xi, n2 - 1.));
        }
        else if ((xi <= 0.) && ((1. - xi) > 0.)) {
            dHfF = 0.5*a1*(n2*pow(1. - xi, n2 - 1.));
        }
        else if ((xi > 0.) && ((1. - xi) <= 0.)) {
            dHfF = 0.5*a1*(n1*pow(xi, n1 - 1.));
        }
        else
        {
            dHfF = 0.;
        }
        
        if ((xi > 0.) && ((1. - xi) > 0.)) {
            dHfR = 0.5*a2*(n3*pow(xi, n3 - 1.) + n4*pow(1. - xi, n4 - 1.));
        }
        else if ((xi <= 0.) && ((1. - xi) > 0.)) {
            dHfR = 0.5*a2*(n4*pow(1. - xi, n4 - 1.));
        }
        else if ((xi > 0.) && ((1. - xi) <= 0.)) {
            dHfR = 0.5*a2*(n3*pow(xi, n3 - 1.));
        }
        else
        {
            dHfR = 0.;
        }
        
        dHcurdsigma = k1*(Hmax - Hmin)*exp(-1.*k1*sigmastar)*eta_stress(sigma);
        
        //Related to forward transformation
        dPhihatFdsigma = dHcurdsigma * Prager_stress(sigma, prager_b, prager_n) + Hcur * dPrager_stress(sigma, prager_b, prager_n);
        dPhihatFdxiF = 0.;
        dPhihatFdxiR = 0.;
        
        dA_xiFdsigma = DM_sig + Dalpha*(T+DT);
        dA_xiFdxiF = -dHfF;
        dA_xiFdxiR = dHfF;
        
        dlambda1dsigma = zeros(6);
        dlambda1dxiF = dlagrange_pow_1(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        dlambda1dxiR = -1.*dlagrange_pow_1(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        
        dYtFdsigma = D*(dHcurdsigma * Mises_stress(sigma) + Hcur * eta_stress(sigma));
        dYtFdxiF = 0.;
        dYtFdxiR = 0.;
        
        dPhiFdsigma = dPhihatFdsigma + dA_xiFdsigma - dlambda1dsigma - dYtFdsigma;
        dPhiFdxiF = dPhihatFdxiF + dA_xiFdxiF - dlambda1dxiF - dYtFdxiF;
        dPhiFdxiR = dPhihatFdxiR + dA_xiFdxiR - dlambda1dxiR - dYtFdxiR;
        
        //Relative to reverse transformation
        dPhihatRdsigma = ETMean;
        dPhihatRdxiF = (-1./xi)*sum(sigma%ETMean);
        dPhihatRdxiR = (1./xi)*sum(sigma%ETMean);
        dPhihatRdETF = sigma/xi;
        dPhihatRdETR = sigma/xi;
        
        dA_xiRdsigma = -1.*DM_sig -1.*Dalpha*(T+DT-T_init);
        dA_xiRdxiF = dHfR;
        dA_xiRdxiR = -dHfR;
        
        dlambda0dsigma = zeros(6);
        dlambda0dxiF = -1.*dlagrange_pow_0(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        dlambda0dxiR = dlagrange_pow_0(xi, c_lambda, p0_lambda, n_lambda, alpha_lambda);
        
        dYtRdsigma = 1.*D*ETMean;
        dYtRdxiF = (-D/xi)*sum(sigma%ETMean);
        dYtRdxiR = (D/xi)*sum(sigma%ETMean);
        dYtRdETF = D*sigma/xi;
        dYtRdETR = D*sigma/xi;
        
        dPhiRdsigma = -1.*dPhihatRdsigma + dA_xiRdsigma + dlambda0dsigma - dYtRdsigma;
        dPhiRdxiF = -1.*dPhihatRdxiF + dA_xiRdxiF + dlambda0dxiF - dYtRdxiF;
        dPhiRdxiR = -1.*dPhihatRdxiR + dA_xiRdxiR + dlambda0dxiR - dYtRdxiR;
        dPhiRdETF = -1.*dPhihatRdETF - dYtRdETF;
        dPhiRdETR = -1.*dPhihatRdETR - dYtRdETR;
        
        K(0,0) = dPhiFdxiF;
        K(0,1) = dPhiFdxiR;
        K(1,0) = dPhiRdxiF + sum(dPhiRdETF%lambdaTF);
        K(1,1) = dPhiRdxiR + sum(dPhiRdETR%lambdaTR);
        
        B(0,0) = -1.*sum(dPhiFdsigma%kappa_j[0]) + K(0,0);
        B(0,1) = -1.*sum(dPhiFdsigma%kappa_j[1]) + K(0,1);
        B(1,0) = -1.*sum(dPhiRdsigma%kappa_j[0]) + K(1,0);
        B(1,1) = -1.*sum(dPhiRdsigma%kappa_j[1]) + K(1,1);
        
        Y_crit(0) = YtF;
        Y_crit(1) = YtR;
        
        Fischer_Burmeister_m(Phi, Y_crit, B, Ds_j, ds_j, error);
        
        s_j(0) += ds_j(0);
        s_j(1) += ds_j(1);
        
        ET = ET + ds_j(0)*lambdaTF + ds_j(1)*lambdaTR;
        
        xiF = s_j(0);
        xiR = s_j(1);
        xi = xi + ds_j(0) - 1.*ds_j(1);
        
        DETF += ds_j(0)*lambdaTF;
        DETR += -1.*ds_j(1)*lambdaTR;
        
        if((Mises_strain(ET) > precision_umat)&&(xi > precision_umat))
        {
            ETMean = dev(ET) / (xi);
        }
        else {
            ETMean = lambdaTF;
        }
        
        //the stress is now computed using the relationship sigma = L(E-Ep)
        Eel = Etot + DEtot - alpha*(T + DT - T_init) - ET;
        sigma = el_pred(L, Eel, ndi);
    }
    
    //Computation of the increments of variables
    vec Dsigma = sigma - sigma_start;
    vec DET = ET - ET_start;
    double DxiF = Ds_j[0];
    double DxiR = Ds_j[1];
    
    double dA_xiFdtheta = rhoDs0 + sum(sigma%Dalpha);
    double dA_xiRdtheta = -1.*rhoDs0 - sum(sigma%Dalpha);
    
    double dPhiFdtheta = dA_xiFdtheta;
    double dPhiRdtheta = dA_xiRdtheta;
    
    
    //Computation of the tangent modulus
    mat Bhat = zeros(2, 2);
    Bhat(0,0) = sum(dPhiFdsigma%kappa_j[0]) - K(0,0);
    Bhat(0,1) = sum(dPhiFdsigma%kappa_j[1]) - K(0,1);
    Bhat(1,0) = sum(dPhiRdsigma%kappa_j[0]) - K(1,0);
    Bhat(1,1) = sum(dPhiRdsigma%kappa_j[1]) - K(1,1);
    
    vec op = zeros(2);
    mat delta = eye(2,2);
    
    for (int i=0; i<2; i++) {
        if(Ds_j[i] > iota)
            op(i) = 1.;
    }
    
    mat Bbar = zeros(2,2);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            Bbar(i, j) = op(i)*op(j)*Bhat(i, j) + delta(i,j)*(1-op(i)*op(j));
        }
    }
    
    mat invBbar = zeros(2, 2);
    mat invBhat = zeros(2, 2);
    invBbar = inv(Bbar);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            invBhat(i, j) = op(i)*op(j)*invBbar(i, j);
        }
    }
    
    std::vector<vec> P_epsilon(2);
    P_epsilon[0] = invBhat(0, 0)*(L*dPhiFdsigma) + invBhat(1, 0)*(L*dPhiRdsigma);
    P_epsilon[1] = invBhat(0, 1)*(L*dPhiFdsigma) + invBhat(1, 1)*(L*dPhiRdsigma);
    
    std::vector<double> P_theta(2);
    P_theta[0] = invBhat(0, 0)*(dPhiFdtheta - sum(dPhiFdsigma%(L*alpha))) + invBhat(1, 0)*(dPhiRdtheta - sum(dPhiRdsigma%(L*alpha)));
    P_theta[1] = invBhat(0, 1)*(dPhiFdtheta - sum(dPhiFdsigma%(L*alpha))) + invBhat(1, 1)*(dPhiRdtheta - sum(dPhiRdsigma%(L*alpha)));

    dSdE = L - (kappa_j[0]*P_epsilon[0].t() + kappa_j[1]*P_epsilon[1].t());
    dSdT = -1.*L*alpha;// - (kappa_j[0]*P_theta[0] + kappa_j[1]*P_theta[1]);

    //Preliminaries for the computation of mechanical and thermal work
    double c_0A = rho*c_pA;
    double c_0M = rho*c_pM;
    double c_0 = c_0A*(1-xi) + c_0M*xi;
    double Dc_0 = c_0M - c_0A;
    
    double eta_r = (c_0A + Dc_0*xi)*log((T+DT)/T_init) + sum(alpha%sigma) + rhoDs0*xi;
    double eta_r_start = (c_0A + Dc_0*xi_start)*log(T/T_init) + sum(alpha_start%sigma_start) + rhoDs0*xi_start;
    
    double eta_ir = 0.;
    double eta_ir_start = 0.;
    
    double eta = eta_r + eta_ir;
    double eta_start = eta_r_start + eta_ir_start;
    
//    double A_theta = eta;
    vec dA_thetadsigma = alpha;
    double dA_thetadxiF = Dc_0*log((T+DT)/T_init) + sum(Dalpha%sigma) + rhoDs0;
    double dA_thetadxiR = -1.*Dc_0*log((T+DT)/T_init) - sum(Dalpha%sigma) - rhoDs0;
//    double dA_thetadtheta = (c_0+Dc_0*xi)/(T+DT);
    
    double Deta = eta - eta_start;
    double Deta_r = eta_r - eta_r_start;
    double Deta_ir = eta_ir - eta_ir_start;
    
    vec Gamma_epsilon = zeros(6);
    double Gamma_theta = 0.;
    
    vec N_epsilon = zeros(6);
    double N_theta = 0.;
    
    if(DTime < 1.E-12) {
        r = 0.;
        drdE = zeros(6);
        drdT = 0.;
    }
    else {
        Gamma_epsilon = (dSdE*DET)*(1./DTime)
        + (dSdE*dA_xiFdsigma + dA_xiFdxiF*P_epsilon[0] + dA_xiFdxiR*P_epsilon[1])*(DxiF/DTime)
        + (dSdE*dA_xiRdsigma + dA_xiRdxiF*P_epsilon[0] + dA_xiRdxiR*P_epsilon[1])*(DxiR/DTime)
        + (A_xiF + sum(sigma%lambdaTF))*P_epsilon[0]/DTime
        + (A_xiR + sum(sigma%lambdaTR))*P_epsilon[1]/DTime;
        
        Gamma_theta = sum(dSdT%DET)*(1./DTime)
        + (dA_xiFdtheta + sum(dA_xiFdsigma%dSdT) + dA_xiFdxiF*P_theta[0] + dA_xiFdxiR*P_theta[1])*(DxiF/DTime)
        + (dA_xiRdtheta + sum(dA_xiRdsigma%dSdT) + dA_xiRdxiF*P_theta[0] + dA_xiRdxiR*P_theta[1])*(DxiR/DTime)
        + (A_xiF + sum(sigma%lambdaTF))*P_theta[0]/DTime
        + (A_xiR + sum(sigma%lambdaTR))*P_theta[1]/DTime;
        
        N_epsilon = -1.*(dSdE*dA_thetadsigma + dA_thetadxiF*P_epsilon[0] + dA_thetadxiR*P_epsilon[1])*((T + DT)/DTime);
        N_theta = -1.*((T+DT)*sum(dSdT%dA_thetadsigma) + Deta + (c_0+Dc_0*xi) + (T+DT)*(dA_thetadxiF*P_theta[0] + dA_thetadxiR*P_theta[1]))*(1./DTime);
        
        drdE = N_epsilon + Gamma_epsilon;
        drdT = N_theta + Gamma_theta;
        
        r = sum(N_epsilon%DEtot) + N_theta*DT + sum(Gamma_epsilon%DEtot) + Gamma_theta*DT;
    }
      
    double Dgamma_loc = 0.5*sum((sigma_start+sigma)%(DET)) + 0.5*(A_xiF_start + A_xiF)*DxiF + 0.5*(A_xiR_start + A_xiR)*DxiR;
    
    //Computation of the mechanical and thermal work quantities
    Wm += 0.5*sum((sigma_start+sigma)%DEtot);
    Wm_r += 0.5*sum((sigma_start+sigma)%(DEtot-DET)) - 0.5*(A_xiF_start + A_xiF)*DxiF - 0.5*(A_xiR_start + A_xiR)*DxiR;
    Wm_ir += 0.;
    Wm_d += Dgamma_loc;
    
    Wt += (T+0.5*DT)*Deta;
    Wt_r += (T+0.5*DT)*Deta_r;
    Wt_ir += (T+0.5*DT)*Deta_ir;
    
    ///@brief statev evolving variables
    statev(0) = T_init;
    statev(1) = xi;
    
    statev(2) = ET(0);
    statev(3) = ET(1);
    statev(4) = ET(2);
    statev(5) = ET(3);
    statev(6) = ET(4);
    statev(7) = ET(5);
    
    statev(8) = xiF;
    statev(9) = xiR;
    
    if(start) {
        statev(10) = rhoDs0;
        statev(11) = rhoDE0;
        statev(12) = D;
        
        statev(13) = a1;
        statev(14) = a2;
        statev(15) = a3;
        
        statev(16) = Y0t;
    }

}

