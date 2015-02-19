/** tutorial/Ex1
 * This example shows how to:
 * initialize a femus application
 * define the multilevel-mesh object mlMsh
 * read from the file ./input/square.neu the coarse-level mesh and associate it to mlMsh   
 * add in mlMsh uniform refined level-meshes 
 * define the multilevel-solution object mlSol associated to mlMsh
 * add in mlSol different types of finite element solution variables
 * initialize the solution varables
 * define vtk and gmv writer objects associated to mlSol  
 * print vtk and gmv binary-format files in ./output directory
 **/

#include "MultiLevelProblem.hpp"
#include "VTKWriter.hpp"
#include "GMVWriter.hpp"
#include "FemusInit.hpp"

using namespace femus;

double InitalValueU(const double &x, const double &y, const double &z){
  return x+y;
}

double InitalValueP(const double &x, const double &y, const double &z){
  return x;
}

double InitalValueT(const double &x, const double &y, const double &z){
  return y;
}

int main(int argc, char **args) {
  
  /// Init Petsc-MPI communicator
  FemusInit mpinit(argc, args, MPI_COMM_WORLD);
 
  // read coarse level mesh and generate finers level meshes
  MultiLevelMesh mlMsh;
  double scalingFactor=1.; 
  mlMsh.ReadCoarseMesh("./input/square.neu","seventh",scalingFactor); 
  /* "seventh" is the order of accuracy that is used in the gauss integration scheme
      probably in the furure it is not going to be an argument of this function   */
  unsigned numberOfUniformLevels=3;
  unsigned numberOfSelectiveLevels=0;
  mlMsh.RefineMesh(numberOfUniformLevels , numberOfUniformLevels + numberOfSelectiveLevels, NULL);
  mlMsh.PrintInfo();
  
  // define and initialize variables
  MultiLevelSolution mlSol(&mlMsh);
  
  mlSol.AddSolution("U",LAGRANGE, FIRST);
  mlSol.AddSolution("V",LAGRANGE, SERENDIPITY);
  mlSol.AddSolution("W",LAGRANGE, SECOND);
  mlSol.AddSolution("P",DISCONTINOUS_POLYNOMIAL, ZERO);
  mlSol.AddSolution("T",DISCONTINOUS_POLYNOMIAL, FIRST);
  
  mlSol.Initialize("All"); // initialize all varaibles to zero
  
  mlSol.Initialize("U", InitalValueU);
  mlSol.Initialize("P", InitalValueP);
  mlSol.Initialize("T", InitalValueT); // note that this initialization is the same as piecewise constant element
     
  // print solutions
  std::vector < std::string > variablesToBePrinted;
  variablesToBePrinted.push_back("U");
  variablesToBePrinted.push_back("P");
  variablesToBePrinted.push_back("T");

  VTKWriter vtkIO(mlSol);
  vtkIO.write_system_solutions(DEFAULT_OUTPUTDIR, "biquadratic", variablesToBePrinted);

  GMVWriter gmvIO(mlSol);
  variablesToBePrinted.push_back("all");
  gmvIO.SetDebugOutput(false);
  gmvIO.write_system_solutions(DEFAULT_OUTPUTDIR, "biquadratic", variablesToBePrinted);
   
  return 0;
}




