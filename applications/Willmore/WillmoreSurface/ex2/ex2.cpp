/** tutorial/Ex3
 * This example shows how to set and solve the weak form of the nonlinear problem
 *                     -\Delta^2 u = f(x) \text{ on }\Omega,
 *            u=0 \text{ on } \Gamma,
 *      \Delat u=0 \text{ on } \Gamma,
 * on a box domain $\Omega$ with boundary $\Gamma$,
 * by using a system of second order partial differential equation.
 * all the coarse-level meshes are removed;
 * a multilevel problem and an equation system are initialized;
 * a direct solver is used to solve the problem.
 **/

#include "FemusInit.hpp"
#include "MultiLevelProblem.hpp"
#include "NumericVector.hpp"
#include "VTKWriter.hpp"
#include "GMVWriter.hpp"
#include "NonLinearImplicitSystem.hpp"
#include "TransientSystem.hpp"
#include "adept.h"
#include <cstdlib>
#include "PetscMatrix.hpp"

const unsigned P = 2.;
using namespace femus;


void AssemblePWillmore(MultiLevelProblem& );

double GetTimeStep (const double time){
  return .01;
}

bool SetBoundaryCondition(const std::vector < double >& x, const char SolName[], double& value, const int facename, const double time) {
  bool dirichlet = true; 
  
  if (!strcmp("Dx1", SolName) || !strcmp("Dx2", SolName) || !strcmp("Dx3", SolName)) {
    value = 0.;
  } 
  else if (!strcmp("Y1", SolName)) {
    value = -2. * x[0];
  }
  else if (!strcmp("Y2", SolName)) {
    value = -2. * x[1];
  }
  else if (!strcmp("Y3", SolName)) {
    value = 0.;
  }
  return dirichlet;
}

double InitalValueY1(const std::vector < double >& x) {
  return -2. * x[0];
}

double InitalValueY2(const std::vector < double >& x) {
  return -2. * x[1];
}

double InitalValueY3(const std::vector < double >& x) {
  return -2. * x[2];
}

int main(int argc, char** args) {

  // init Petsc-MPI communicator
  FemusInit mpinit(argc, args, MPI_COMM_WORLD);


  // define multilevel mesh


  unsigned maxNumberOfMeshes;
 
  MultiLevelMesh mlMsh;
  // read coarse level mesh and generate finers level meshes
  double scalingFactor = 1.;
  
  mlMsh.ReadCoarseMesh("./input/sphere.neu", "seventh", scalingFactor);
  
  unsigned numberOfUniformLevels = 1;
  unsigned numberOfSelectiveLevels = 0;
  mlMsh.RefineMesh(numberOfUniformLevels , numberOfUniformLevels + numberOfSelectiveLevels, NULL);

  // erase all the coarse mesh levels
  mlMsh.EraseCoarseLevels(numberOfUniformLevels - 1);

  // print mesh info
  mlMsh.PrintInfo();

  // define the multilevel solution and attach the mlMsh object to it
  MultiLevelSolution mlSol(&mlMsh);

    // add variables to mlSol
  mlSol.AddSolution("Dx1", LAGRANGE, SECOND, 2);
  mlSol.AddSolution("Dx2", LAGRANGE, SECOND, 2);
  mlSol.AddSolution("Dx3", LAGRANGE, SECOND, 2);
  
//   mlSol.AddSolution("Dx1", LAGRANGE, FIRST, 0);
//   mlSol.AddSolution("Dx2", LAGRANGE, FIRST, 0);
//   mlSol.AddSolution("Dx3", LAGRANGE, FIRST, 0);
//   
  mlSol.AddSolution("Y1", LAGRANGE, SECOND, 0);
  mlSol.AddSolution("Y2", LAGRANGE, SECOND, 0);
  mlSol.AddSolution("Y3", LAGRANGE, SECOND, 0);
  
//   mlSol.AddSolution("Y1", LAGRANGE, FIRST, 0);
//   mlSol.AddSolution("Y2", LAGRANGE, FIRST, 0);
//   mlSol.AddSolution("Y3", LAGRANGE, FIRST, 0);

  
  mlSol.Initialize("All");
  mlSol.Initialize("Y1", InitalValueY1);
  mlSol.Initialize("Y2", InitalValueY2);
  mlSol.Initialize("Y3", InitalValueY3);
  
 
  mlSol.AttachSetBoundaryConditionFunction(SetBoundaryCondition);
  mlSol.GenerateBdc("All");
  
//   mlSol.FixSolutionAtOnePoint("Dx1");
//   mlSol.FixSolutionAtOnePoint("Dx2");
//   mlSol.FixSolutionAtOnePoint("Dx3");
//   mlSol.FixSolutionAtOnePoint("Y1");
//   mlSol.FixSolutionAtOnePoint("Y2");
//   mlSol.FixSolutionAtOnePoint("Y3");
   

  MultiLevelProblem mlProb(&mlSol);
  
  // add system Wilmore in mlProb as a Linear Implicit System
  TransientNonlinearImplicitSystem& system = mlProb.add_system < TransientNonlinearImplicitSystem > ("PWillmore");
  
  // add solution "X", "Y", "Z" and "H" to the system
  system.AddSolutionToSystemPDE("Dx1");
  system.AddSolutionToSystemPDE("Dx2");
  system.AddSolutionToSystemPDE("Dx3");
  system.AddSolutionToSystemPDE("Y1");
  system.AddSolutionToSystemPDE("Y2");
  system.AddSolutionToSystemPDE("Y3");
  
  system.SetMaxNumberOfNonLinearIterations(10);
  
  // attach the assembling function to system
  system.SetAssembleFunction(AssemblePWillmore);
  
  system.AttachGetTimeIntervalFunction(GetTimeStep);
  
  // initilaize and solve the system
  system.init();
  
 
    
  mlSol.SetWriter(VTK);
  std::vector<std::string> mov_vars;
  mov_vars.push_back("Dx1");
  mov_vars.push_back("Dx2");
  mov_vars.push_back("Dx3");  
  mlSol.GetWriter()->SetMovingMesh(mov_vars);
  
  std::vector < std::string > variablesToBePrinted;
  variablesToBePrinted.push_back("All");

  mlSol.GetWriter()->SetDebugOutput(true);
  mlSol.GetWriter()->Write(DEFAULT_OUTPUTDIR, "biquadratic", variablesToBePrinted, 0);
  
  unsigned numberOfTimeSteps = 1000;
  for(unsigned time_step = 0; time_step < numberOfTimeSteps; time_step++){
    
    system.CopySolutionToOldSolution();
    system.MGsolve();
    
    mlSol.GetWriter()->Write(DEFAULT_OUTPUTDIR, "biquadratic", variablesToBePrinted, time_step + 1);
  }
    
  return 0;
}

void AssemblePWillmore(MultiLevelProblem& ml_prob) {
  //  ml_prob is the global object from/to where get/set all the data
  //  level is the level of the PDE system to be assembled
  //  levelMax is the Maximum level of the MultiLevelProblem
  //  assembleMatrix is a flag that tells if only the residual or also the matrix should be assembled
  
  // call the adept stack object
  adept::Stack& s = FemusInit::_adeptStack;
  
  //  extract pointers to the several objects that we are going to use
  TransientNonlinearImplicitSystem* mlPdeSys   = &ml_prob.get_system<TransientNonlinearImplicitSystem> ("PWillmore");   // pointer to the linear implicit system named "Poisson"
  
  double dt = GetTimeStep(0);
  
  const unsigned level = mlPdeSys->GetLevelToAssemble();
  
  Mesh *msh = ml_prob._ml_msh->GetLevel(level);    // pointer to the mesh (level) object
  elem *el = msh->el;  // pointer to the elem object in msh (level)
  
  MultiLevelSolution *mlSol = ml_prob._ml_sol;  // pointer to the multilevel solution object
  Solution *sol = ml_prob._ml_sol->GetSolutionLevel(level);    // pointer to the solution (level) object
  
  LinearEquationSolver *pdeSys = mlPdeSys->_LinSolver[level]; // pointer to the equation (level) object
  SparseMatrix *KK = pdeSys->_KK;  // pointer to the global stifness matrix object in pdeSys (level)
  NumericVector *RES = pdeSys->_RES; // pointer to the global residual vector object in pdeSys (level)
  
  const unsigned  dim = 2; 
  const unsigned  DIM = 3; 
  unsigned iproc = msh->processor_id(); // get the process_id (for parallel computation)
  
  //solution variable
  unsigned solDxIndex[DIM];
  solDxIndex[0] = mlSol->GetIndex("Dx1");  // get the position of "DX" in the ml_sol object
  solDxIndex[1] = mlSol->GetIndex("Dx2");  // get the position of "DY" in the ml_sol object
  solDxIndex[2] = mlSol->GetIndex("Dx3");  // get the position of "DZ" in the ml_sol object
  
  unsigned solxType;
  solxType = mlSol->GetSolutionType(solDxIndex[0]);   // get the finite element type for "U"
    
  unsigned solDxPdeIndex[DIM];
  solDxPdeIndex[0] = mlPdeSys->GetSolPdeIndex("Dx1");    // get the position of "DX" in the pdeSys object
  solDxPdeIndex[1] = mlPdeSys->GetSolPdeIndex("Dx2");    // get the position of "DY" in the pdeSys object
  solDxPdeIndex[2] = mlPdeSys->GetSolPdeIndex("Dx3");    // get the position of "DZ" in the pdeSys object
  
  std::vector < adept::adouble > solx[DIM];  // surface coordinates
  std::vector < double > solxOld[DIM];  // surface coordinates
  unsigned xType = 2; // get the finite element type for "x", it is always 2 (LAGRANGE QUADRATIC)
   
  unsigned solYIndex[DIM];
  solYIndex[0] = mlSol->GetIndex("Y1");    // get the position of "Y1" in the ml_sol object
  solYIndex[1] = mlSol->GetIndex("Y2");    // get the position of "Y2" in the ml_sol object
  solYIndex[2] = mlSol->GetIndex("Y3");    // get the position of "Y3" in the ml_sol object
  
  unsigned solYType;
  solYType = mlSol->GetSolutionType(solYIndex[0]);   // get the finite element type for "Y"
  
  unsigned solYPdeIndex[DIM];
  solYPdeIndex[0] = mlPdeSys->GetSolPdeIndex("Y1");    // get the position of "Y1" in the pdeSys object
  solYPdeIndex[1] = mlPdeSys->GetSolPdeIndex("Y2");    // get the position of "Y2" in the pdeSys object
  solYPdeIndex[2] = mlPdeSys->GetSolPdeIndex("Y3");    // get the position of "Y3" in the pdeSys object
  
  std::vector < adept::adouble > solY[DIM]; // local Y solution
  std::vector< int > SYSDOF; // local to global pdeSys dofs
 
  vector< double > Res; // local redidual vector
  std::vector< adept::adouble > aResx[3]; // local redidual vector
  std::vector< adept::adouble > aResY[3]; // local redidual vector
    
  vector < double > Jac; // local Jacobian matrix (ordered by column, adept)
     
  KK->zero();  // Set to zero all the entries of the Global Matrix
  RES->zero(); // Set to zero all the entries of the Global Residual
  
  // element loop: each process loops only on the elements that owns
  for (int iel = msh->_elementOffset[iproc]; iel < msh->_elementOffset[iproc + 1]; iel++) {
    
    short unsigned ielGeom = msh->GetElementType(iel);
    unsigned nxDofs  = msh->GetElementDofNumber(iel, solxType);    // number of solution element dofs
    unsigned nYDofs  = msh->GetElementDofNumber(iel, solYType);    // number of solution element dofs
       
    for (unsigned K = 0; K < DIM; K++) {
      solx[K].resize(nxDofs);
      solxOld[K].resize(nxDofs);
      solY[K].resize(nYDofs);
    }
    
    // resize local arrays
    SYSDOF.resize( DIM * ( nxDofs + nYDofs ) );
    
    Res.resize( DIM * ( nxDofs + nYDofs ));    //resize
    
    for (unsigned K = 0; K < DIM; K++) {
      aResx[K].assign(nxDofs,0.);    //resize and set to zero
      aResY[K].assign(nYDofs,0.);    //resize and zet to zero
    }
        
    // local storage of global mapping and solution
    for (unsigned i = 0; i < nxDofs; i++) {
      unsigned iDDof = msh->GetSolutionDof(i, iel, solxType); // global to local mapping between solution node and solution dof
      unsigned iXDof  = msh->GetSolutionDof(i, iel, xType);
      
      for (unsigned K = 0; K < DIM; K++) {
        solxOld[K][i] = (*msh->_topology->_Sol[K])(iXDof) + (*sol->_SolOld[solDxIndex[K]])(iDDof);
        solx[K][i] = (*msh->_topology->_Sol[K])(iXDof) + (*sol->_Sol[solDxIndex[K]])(iDDof);
        SYSDOF[K * nxDofs + i] = pdeSys->GetSystemDof(solDxIndex[K], solDxPdeIndex[K], i, iel);  // global to global mapping between solution node and pdeSys dof
      }
    }
    
    // local storage of global mapping and solution
    for (unsigned i = 0; i < nYDofs; i++) {
      unsigned iYDof = msh->GetSolutionDof(i, iel, solYType); // global to local mapping between solution node and solution dof
      for (unsigned K = 0; K < DIM; K++) {
        solY[K][i] = (*sol->_Sol[solYIndex[K]])(iYDof);  // global to local solution
        SYSDOF[DIM * nxDofs + K * nYDofs + i] = pdeSys->GetSystemDof(solYIndex[K], solYPdeIndex[K], i, iel);  // global to global mapping between solution node and pdeSys dof
      }
    }
  
    // start a new recording of all the operations involving adept::adouble variables
    s.new_recording();
        
    // *** Gauss point loop ***
    for (unsigned ig = 0; ig < msh->_finiteElement[ielGeom][solxType]->GetGaussPointNumber(); ig++) {
      
      double *phix;  // local test function
      double *phix_uv[dim]; // local test function first order partial derivatives
            
      double *phiY;  // local test function
      double *phiY_uv[dim]; // local test function first order partial derivatives
      
      double weight; // gauss point weight
            
      // *** get gauss point weight, test function and test function partial derivatives ***
      phix = msh->_finiteElement[ielGeom][solxType]->GetPhi(ig);
      phix_uv[0] = msh->_finiteElement[ielGeom][solxType]->GetDPhiDXi(ig); //derivative in u
      phix_uv[1] = msh->_finiteElement[ielGeom][solxType]->GetDPhiDEta(ig); //derivative in v
      
      phiY = msh->_finiteElement[ielGeom][solYType]->GetPhi(ig);
      phiY_uv[0] = msh->_finiteElement[ielGeom][solYType]->GetDPhiDXi(ig);
      phiY_uv[1] = msh->_finiteElement[ielGeom][solYType]->GetDPhiDEta(ig);
            
      weight = msh->_finiteElement[ielGeom][solxType]->GetGaussWeight(ig);
      
      adept::adouble solx_uv[3][2] = {{0.,0.},{0.,0.},{0.,0.}};
      adept::adouble solY_uv[3][2] = {{0.,0.},{0.,0.},{0.,0.}};
      adept::adouble solYg[3]={0.,0.,0.};
      adept::adouble solxg[3]={0.,0.,0.};
      double solxOldg[3]={0.,0.,0.};
      
      for(unsigned K = 0; K < DIM; K++){
        for (unsigned i = 0; i < nxDofs; i++) {
          solxg[K] += phix[i] * solx[K][i];
          solxOldg[K] += phix[i] * solxOld[K][i];
        }  
        for (unsigned i = 0; i < nYDofs; i++) {
          solYg[K] += phiY[i] * solY[K][i];
        }
        for (int j = 0; j < dim; j++) {
          for (unsigned i = 0; i < nxDofs; i++) {
            solx_uv[K][j] += phix_uv[j][i] * solx[K][i];
          }
        }
        for (int j = 0; j < dim; j++) {
          for (unsigned i = 0; i < nYDofs; i++) {
            solY_uv[K][j] += phiY_uv[j][i] * solY[K][i];
          }
        }
        
      }
      
      adept::adouble solYnorm2 = 0.;
      for(unsigned K = 0; K < DIM; K++){
        solYnorm2 += solYg[K] * solYg[K];
      }
//       std::cout << solYnorm2 << " ";
            
      adept::adouble g[dim][dim]={{0.,0.},{0.,0.}};
      for(unsigned i = 0; i < dim; i++){
        for(unsigned j = 0; j < dim; j++){
          for(unsigned K = 0; K < DIM; K++){
            g[i][j] += solx_uv[K][i] * solx_uv[K][j];
          }
        }
      }
      adept::adouble detg = g[0][0] * g[1][1] - g[0][1] * g[1][0];
      adept::adouble gi[dim][dim];
      gi[0][0] =  g[1][1]/detg;
      gi[0][1] = -g[0][1]/detg;
      gi[1][0] = -g[1][0]/detg;
      gi[1][1] =  g[0][0]/detg;
      
      adept::adouble Area = weight * sqrt(detg);
      
      adept::adouble Jir[2][3]={{0.,0.,0.},{0.,0.,0.}};
      for(unsigned i = 0; i < dim; i++){
        for(unsigned J = 0; J < DIM; J++){
          for(unsigned k = 0; k < dim; k++){
            Jir[i][J] += gi[i][k] * solx_uv[J][k];
          }
        }
      }
      
      adept::adouble solx_Xtan[DIM][DIM]={{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}};
      adept::adouble solY_Xtan[DIM][DIM]={{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}};
      
      
      for(unsigned I = 0; I < DIM; I++){
        for(unsigned J = 0; J < DIM; J++){
          for(unsigned k = 0; k < dim; k++){
            solx_Xtan[I][J] += solx_uv[I][k] * Jir[k][J];
            solY_Xtan[I][J] += solY_uv[I][k] * Jir[k][J];
          }
        }
      }
      
      
      std::vector < adept::adouble > phiY_Xtan[DIM];
      std::vector < adept::adouble > phix_Xtan[DIM];
      
      for(unsigned J = 0; J < DIM; J++){
        phix_Xtan[J].assign(nxDofs,0.);
        for(unsigned inode  = 0; inode < nxDofs; inode++){
          for(unsigned k = 0; k < dim; k++){
            phix_Xtan[J][inode] += phix_uv[k][inode] * Jir[k][J];
          }
        }
                
        phiY_Xtan[J].assign(nYDofs,0.);
        for(unsigned inode  = 0; inode < nYDofs; inode++){
          for(unsigned k = 0; k < dim; k++){
            phiY_Xtan[J][inode] += phiY_uv[k][inode] * Jir[k][J];
          }
        }
      }
            
      for(unsigned K = 0; K < DIM; K++){
        for(unsigned i = 0; i < nxDofs; i++){
          aResx[K][i] -= ( solYg[K] * phix[i] + phix_Xtan[K][i]) * Area; 
        }
        for(unsigned i = 0; i < nYDofs; i++){
          adept::adouble term1 = - solYnorm2;
          adept::adouble term2 = 0.;
          adept::adouble term3 = 0.;
          for(unsigned J = 0; J < DIM; J++){
            term1 -= P * solY_Xtan[J][J];
            term2 +=  P * solY_Xtan[K][J] * phiY_Xtan[J][i]; 
            adept::adouble term4 = 0.;
            for(unsigned L = 0; L < DIM; L++){
              term4 += solx_Xtan[J][L] * solY_Xtan[K][L] + solx_Xtan[K][L] * solY_Xtan[J][L];
            }
            term3 += P * phiY_Xtan[J][i] * term4;
          }
          aResY[K][i] -= ( -(solxg[K] - solxOldg[K]) / dt * phiY[i]  + term1 * phiY_Xtan[K][i] - term2 + term3 ) * Area; 
        }
      }
    } // end gauss point loop
    
    //--------------------------------------------------------------------------------------------------------
    // Add the local Matrix/Vector into the global Matrix/Vector
    
    //copy the value of the adept::adoube aRes in double Res and store
    
    for (int K = 0; K < DIM; K++) {
      for (int i = 0; i < nxDofs; i++) {
        Res[ K * nxDofs + i] = -aResx[K][i].value();
        //std::cout << SYSDOF[K * nxDofs + i]<<"( " << Res[ K * nxDofs + i] <<" ) ";
      }
      for (int i = 0; i < nYDofs; i++) {
        Res[DIM * nxDofs + K * nYDofs + i] = -aResY[K][i].value();
        //std::cout << SYSDOF[DIM * nxDofs + K * nYDofs + i]<<"( " << Res[ DIM * nxDofs + K * nYDofs + i] <<" ) ";
      }
      //std::cout << "\n ";
    }
    
    
    
    RES->add_vector_blocked(Res, SYSDOF);
    
    
    Jac.resize(DIM * (nxDofs + nYDofs) * DIM * (nxDofs + nYDofs) );
    
    // define the dependent variables
    for (int K = 0; K < DIM; K++) {
      s.dependent(&aResx[K][0], nxDofs);
    }
    for (int K = 0; K < DIM; K++) {
      s.dependent(&aResY[K][0], nYDofs);
    }
     
    // define the dependent variables
    for (int K = 0; K < DIM; K++) {
      s.independent(&solx[K][0], nxDofs);
    }
    for (int K = 0; K < DIM; K++) {
      s.independent(&solY[K][0], nYDofs);
    } 
        
    // get the jacobian matrix (ordered by row)
    s.jacobian(&Jac[0], true);
    
    KK->add_matrix_blocked(Jac, SYSDOF, SYSDOF);
    
    s.clear_independents();
    s.clear_dependents();
    
  } //end element loop for each process
  
  RES->close();
  KK->close();
  
 // VecView((static_cast<PetscVector*>(RES))->vec(),	PETSC_VIEWER_STDOUT_SELF );
  
  
 // MatView((static_cast<PetscMatrix*>(KK))->mat(), PETSC_VIEWER_STDOUT_SELF );
  
//     PetscViewer    viewer;
//     PetscViewerDrawOpen(PETSC_COMM_WORLD,NULL,NULL,0,0,900,900,&viewer);
//     PetscObjectSetName((PetscObject)viewer,"FSI matrix");
//     PetscViewerPushFormat(viewer,PETSC_VIEWER_DRAW_LG);
//     MatView((static_cast<PetscMatrix*>(KK))->mat(),viewer);
//     double a;
//     std::cin>>a;
  
  
  // ***************** END ASSEMBLY *******************
}

