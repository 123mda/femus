/*=========================================================================

 Program: FEMUS
 Module: GambitIO
 Authors: Eugenio Aulisa, Simone Bnà
 
 Copyright (c) FEMTTU
 All rights reserved. 

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __GambitIO_hpp__
#define __GambitIO_hpp__


// Local includes
#include "MeshInput.hpp"

namespace femus
{

// Forward declarations
class mesh;

/**
 * This class implements writing meshes in the Gmsh format.
 */

// ------------------------------------------------------------
// GMVIO class definition
class GambitIO : public MeshInput<mesh>
{
 public:

  /**
   * Constructor.  Takes a non-const Mesh reference which it
   * will fill up with elements via the read() command.
   */
  explicit
  GambitIO (mesh& mesh);

  /**
   * Reads in a mesh in the neutral gambit *.neu format
   * from the ASCII file given by name.
   *
   */
  virtual void read (const std::string& name, vector < vector < double> > &vt, const double Lref, std::vector<bool> &type_elem_flag);

};


inline
GambitIO::GambitIO (mesh& meshinput) :
   MeshInput<mesh>  (meshinput)
{
}


} // namespace femus

#endif 