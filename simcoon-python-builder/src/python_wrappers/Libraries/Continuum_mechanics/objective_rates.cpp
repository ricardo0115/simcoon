
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <string>
#include <carma>
#include <armadillo>

#include <simcoon/Continuum_mechanics/Functions/objective_rates.hpp>
#include <simcoon/python_wrappers/Libraries/Continuum_mechanics/objective_rates.hpp>
#include <simcoon/Continuum_mechanics/Functions/transfer.hpp>

using namespace std;
using namespace arma;
namespace py=pybind11;

namespace simpy {

//This function computes the logarithmic strain velocity and the logarithmic spin, along with the correct rotation increment
py::tuple logarithmic(const py::array_t<double> &F0, const py::array_t<double> &F1, const double &DTime, const bool &copy) {
    mat F0_cpp = carma::arr_to_mat(F0);
    mat F1_cpp = carma::arr_to_mat(F1);
    mat DR = zeros(3,3);
    mat D = zeros(3,3);
    mat Omega = zeros(3,3);
    simcoon::logarithmic(DR, D, Omega, DTime, F0_cpp, F1_cpp);
    return py::make_tuple(carma::mat_to_arr(D, copy), carma::mat_to_arr(DR, copy), carma::mat_to_arr(Omega, copy));
}


//This function computes the logarithmic strain velocity and the logarithmic spin, along with the correct rotation increment
py::tuple objective_rate(const std::string& corate_name, const py::array_t<double> &F0, const py::array_t<double> &F1, const double &DTime, const bool &return_de) {
    std::map<string, int> list_corate;
    list_corate = { {"jaumann",0},{"green_naghdi",1},{"logarithmic",2}, {"gn",1},{"log",2}};
	int corate = list_corate[corate_name];
    void (*corate_function)(mat &, mat &, mat &, const double &, const mat &, const mat &); 
    switch (corate) {

        case 0: {
            corate_function = &simcoon::Jaumann;
            break;
        }
        case 1: {
            corate_function = &simcoon::Green_Naghdi;
            break;
        }
        case 2: {
            corate_function = &simcoon::logarithmic;
            break;
        }
    }
    
    if (F1.ndim() == 2) {            
        if (F0.ndim() != 2) {
            throw std::invalid_argument("the number of dim of F1 should be the same as F0");
        }

        mat F0_cpp = carma::arr_to_mat(F0);
        mat F1_cpp = carma::arr_to_mat(F1);
        mat DR = zeros(3,3);
        mat D = zeros(3,3);
        mat Omega = zeros(3,3); 

        corate_function(DR, D, Omega, DTime, F0_cpp, F1_cpp);
        if (return_de) {
            //also return the strain increment
            vec de = (0.5*DTime)*simcoon::t2v_strain((D+(DR*D*DR.t())));
            //could use simcoon::Delta_log_strain(D, Omega, DTime) but it would recompute DR (waste of time).
            //vec de = simcoon::t2v_strain(simcoon::Delta_log_strain(D, Omega, DTime)); 
            return py::make_tuple(carma::col_to_arr(de,false), carma::mat_to_arr(D, false), carma::mat_to_arr(DR, false), carma::mat_to_arr(Omega, false));
        }
        else{
            return py::make_tuple(carma::mat_to_arr(D, false), carma::mat_to_arr(DR, false), carma::mat_to_arr(Omega, false));
        }
        
    }
    else if (F1.ndim() == 3) {
        cube F1_cpp = carma::arr_to_cube_view(F1);            
        int nb_points = F1_cpp.n_slices;
        cube DR(3,3,nb_points);
        cube D(3,3,nb_points);            
        cube Omega = zeros(3,3, nb_points); 			

        if (F0.ndim() == 2) {
            mat vec_F0 = carma::arr_to_mat_view(F0);
            for (int pt = 0; pt < nb_points; pt++) {
                corate_function(DR.slice(pt), D.slice(pt), Omega.slice(pt), DTime, vec_F0, F1_cpp.slice(pt));            
            }
        }
        else if (F0.ndim() == 3) {
            cube F0_cpp = carma::arr_to_cube_view(F0); 
            if (F0_cpp.n_slices==1) {
                mat vec_F0 = F0_cpp.slice(0);
                for (int pt = 0; pt < nb_points; pt++) {
                    corate_function(DR.slice(pt), D.slice(pt), Omega.slice(pt), DTime, vec_F0, F1_cpp.slice(pt));                                
                }
            }
            else {
                for (int pt = 0; pt < nb_points; pt++) {
                    corate_function(DR.slice(pt), D.slice(pt), Omega.slice(pt), DTime, F0_cpp.slice(pt), F1_cpp.slice(pt));            
                }
            }
        }
        if (return_de){
            //also return the strain increment
            mat de(6,nb_points);
            for (int pt = 0; pt < nb_points; pt++) {
                de.col(pt) = (0.5*DTime) * simcoon::t2v_strain(D.slice(pt)+(DR.slice(pt)*D.slice(pt)*DR.slice(pt).t()));
            }            
            return py::make_tuple(carma::mat_to_arr(de, false), carma::cube_to_arr(D, false), carma::cube_to_arr(DR, false), carma::cube_to_arr(Omega, false));
        }
        else{
            return py::make_tuple(carma::cube_to_arr(D, false), carma::cube_to_arr(DR, false), carma::cube_to_arr(Omega, false));
        }        
    }
}

//This function computes the gradient of displacement (Eulerian) from the deformation gradient 
py::array_t<double> Delta_log_strain(const py::array_t<double> &D, const py::array_t<double> &Omega, const double &DTime, const bool &copy) {
    mat D_cpp = carma::arr_to_mat(D);
    mat Omega_cpp = carma::arr_to_mat(Omega);
    mat Delta_log_strain = simcoon::Delta_log_strain(D_cpp, Omega_cpp, DTime);
    return carma::mat_to_arr(Delta_log_strain, copy);
}
    
} //namepsace simpy
