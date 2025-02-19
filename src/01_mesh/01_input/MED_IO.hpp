/*=========================================================================

 Program: FEMUS
 Module: MED_IO
 Authors: Giorgio Bornia, Sureka Pathmanathan
 
 Copyright (c) FEMTTU
 All rights reserved. 

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __femus_mesh_MED_IO_hpp__
#define __femus_mesh_MED_IO_hpp__

// Local includes
#include "MeshInput.hpp"
#include "GeomElTypeEnum.hpp"
#include "GeomElemBase.hpp"
#include "GeomElemHex27.hpp"
#include "GeomElemTet10.hpp"
#include "GeomElemQuad9.hpp"
#include "GeomElemTri6.hpp"
#include "GeomElemEdge3.hpp"

#ifdef HAVE_HDF5

#include "hdf5.h"

namespace femus
{

 #define TYPE_FOR_INT_DATASET  int  //do not use "unsigned": in fact, in MED 
                                  // these numbers are NEGATIVE for elements,
                                  //    while they are POSITIVE for nodes!
 #define TYPE_FOR_REAL_DATASET  double

    
// Auxiliary struct to store group information
  struct GroupInfo {
      std::string        _user_defined_group_string;
      std::string        _node_or_cell_group;
      TYPE_FOR_INT_DATASET _med_flag;
      int                _user_defined_flag;
      unsigned int       _user_defined_property;
      GeomElemBase*      _geom_el;
      int _size;
  };
    
// Forward declarations
class Mesh;


/**
 * This class implements reading meshes in the MED format.
 */

// ------------------------------------------------------------
class MED_IO : public MeshInput<Mesh>
{
 public:

  /**
   * Constructor.  Takes a non-const Mesh reference which it
   * will fill up with elements via the read() command.
   */
  explicit
  MED_IO (Mesh& mesh);
  
  ~MED_IO();
  
  /**
   * Reads in a mesh in the  *.med format
   */
  virtual void read (const std::string& name, vector < vector < double> > &coords, const double Lref, std::vector<bool> &type_elem_flag, const bool read_groups, const bool read_boundary_groups);
  
  void boundary_of_boundary_3d_via_nodes(const std::string& name, const unsigned group_user);

  std::vector< TYPE_FOR_REAL_DATASET >  node_based_flag_read_from_file(const std::string& name, const std::vector< unsigned > & mapping);

  static bool boundary_of_boundary_3d_check_face_of_face_via_nodes(const std::vector < int > nodes_face_face_flags, const unsigned group_salome);
  
 private:
     
  void node_read_flag(const hid_t&  file_id, const std::string mesh_menu,  vector < TYPE_FOR_REAL_DATASET >  & node_group_map);
  
  hid_t open_mesh_file(const std::string& name);
  
  void close_mesh_file(hid_t file_id);
  
  std::string get_element_info_all_dims_H5Group(const std::string mesh_menu) const;
  
  std::string get_node_info_H5Group(const std::string mesh_menu) const;
   
  std::string get_group_info_H5Group(const std::string mesh_menu,  const std::string geom_elem_type) const;
 
  hsize_t  get_H5G_size(const hid_t&  gid) const;
   
  std::string  get_H5L_name_by_idx(const hid_t&  loc_id, const char *group_name, const unsigned j) const;
   
 template < class DATASET_TYPE >  
  void dataset_open_and_close_store_in_vector(hid_t file_id, std::vector< DATASET_TYPE > & fam_map, const std::string fam_name_dir_i) const;
  
   unsigned int get_user_flag_from_med_flag(const std::vector< GroupInfo > & group_info, const TYPE_FOR_INT_DATASET med_flag_in ) const;
   
   unsigned int get_med_flag_from_user_flag(const std::vector< GroupInfo > & group_info, const TYPE_FOR_INT_DATASET input_flag) const;

   void set_elem_group_ownership(const hid_t&  file_id,
                                 const std::string mesh_menu,
                                 const std::vector<GroupInfo> & group_info,
                                 const GeomElemBase* geom_elem_per_dimension,
                                 const int i );
   
    void set_elem_group_ownership_boundary(const hid_t  file_id,
                                           const std::string mesh_menu,
                                           const std::vector< GroupInfo > & group_info,
                                           const std::vector< GeomElemBase* > geom_elem_per_dimension,
                                           const unsigned mesh_dim,
                                           MyMatrix <int> & element_faces_array
                              );
                                      
   void compute_group_geom_elem_and_size(const hid_t&  file_id, const std::string mesh_menu, GroupInfo & group_info)  const;

   void set_elem_connectivity(const hid_t&  file_id, const std::string mesh_menu, const unsigned i, const GeomElemBase* geom_elem_per_dimension, std::vector<bool>& type_elem_flag);
   
   void find_boundary_faces_and_set_face_flags(const hid_t&  file_id, 
                                               const std::string mesh_menu, 
                                               const std::vector<GroupInfo> & group_info, const GeomElemBase* geom_elem_per_dimension,
                                               MyMatrix <int> & element_faces_array);

   void find_boundary_nodes_and_set_node_flags(const hid_t&  file_id, 
                                               const std::string mesh_menu, 
                                               const std::vector<GroupInfo> & group_info,
                                               MyMatrix <int> & element_faces_array);
   
  void check_all_boundaries_have_some_condition(const unsigned int count_found_face, const unsigned int fam_size) const;

   
  bool  see_if_faces_from_different_lists_are_the_same( const GeomElemBase* geom_elem_per_dimension, 
                                            const std::vector< unsigned > & face_nodes_from_vol_connectivity, 
                                            const std::vector< unsigned > & face_nodes_from_bdry_group);
  
  void set_node_coordinates(const hid_t&  file_id, const std::string mesh_menu, vector < vector < double> >& coords, const double Lref);

   const GroupInfo                get_group_flags_per_mesh(const std::string & group_names, const std::string  geom_elem_type) const;
   
   const std::vector< GroupInfo > get_all_groups_per_mesh(const hid_t &  file_id, const std::string & mesh_menu) const;
   
   const std::vector<std::string>  get_mesh_names(const hid_t & file_id) const;
     
   std::pair<int, std::vector<int> >  isolate_number_in_string_between_underscores(const std::string & string_in, const int begin_pos_to_investigate) const;
      
   std::string  isolate_first_field_before_underscore(const std::string &  string_in, const int begin_pos_to_investigate) const;

   /** Determine mesh dimension from mesh file. It cannot be const because it sets the dimension in the mesh */
   const std::vector< GeomElemBase* >  set_mesh_dimension_and_get_geom_elems_by_looping_over_element_types(const hid_t &  file_id, const std::string & menu_name);

   GeomElemBase * get_geom_elem_from_med_name(const  std::string el_type) const;

   /** Read FE type @todo this should be const */
  const std::vector< GeomElemBase* > get_geom_elem_type_per_dimension(const hid_t & file_id, const std::string my_mesh_name_dir);
   
   /** Map from Salome vertex index to Femus vertex index */
   static const unsigned MEDToFemusVertexIndex[N_GEOM_ELS][MAX_EL_N_NODES]; 
 
   /** Map from Salome face index to Femus face index */
   static const unsigned MEDToFemusFaceIndex[N_GEOM_ELS][MAX_EL_N_FACES];

//    std::vector<char*> menu_names;
   static const std::string mesh_ensemble;             //ENS_MAA
   static const std::string aux_zeroone;               // -0000000000000000001-0000000000000000001
   static const std::string elem_types_folder;         //MAI
   static const std::string group_fam;                 //FAM
   static const std::string elems_connectivity;              //NOD    //These are written based on the NOE/NUM numbering !
   static const std::string node_or_elem_salome_gui_global_num;   //NUM    //this is the MED global numbering (as you see in Salome) both for nodes (in NOE) and for elements of all dimensions (in MAI). 
                                                       //Salome global Numbering of both Nodes and Elements starts at 1.
                                                       //For Elements, lower dimensional elements are numbered first
   static const std::string nodes_folder;                 //NOE
   static const std::string nodes_coord_list;                //COO
   static const std::string group_ensemble;            //FAS
   static const std::string group_elements;            //ELEME
   static const std::string group_nodes;               //NOEUD  //if you ever use group of nodes, not used now
   static const uint max_length;

   std::vector< GeomElemBase* > _geom_elems;

   bool _print_info;
   
};

inline
MED_IO::MED_IO (Mesh& mesh) :
   MeshInput<Mesh>  (mesh)
{
    
       _geom_elems.resize(N_GEOM_ELS);
       
       _geom_elems[0] = new GeomElemHex27();
       _geom_elems[1] = new GeomElemTet10();
//        _geom_elems[2] = new GeomElemWedge18();
       _geom_elems[3] = new GeomElemQuad9();
       _geom_elems[4] = new GeomElemTri6();
       _geom_elems[5] = new GeomElemEdge3();
    
    _print_info = false;
      
}

inline
MED_IO::~MED_IO () {
  
       delete _geom_elems[0];
       delete _geom_elems[1];
//     delete _geom_elems[2];
       delete _geom_elems[3];
       delete _geom_elems[4];
       delete _geom_elems[5];

    _geom_elems.resize(0);
    
}

   
//template specialization in h file, explicit instantiation in cpp file
 template < >  
  void MED_IO::dataset_open_and_close_store_in_vector<TYPE_FOR_INT_DATASET>(hid_t file_id, std::vector< TYPE_FOR_INT_DATASET > & fam_map, const std::string fam_name_dir_i) const  {
      
       hid_t dtset_fam            = H5Dopen(file_id, fam_name_dir_i.c_str(), H5P_DEFAULT);
      hid_t filespace_fam        = H5Dget_space(dtset_fam);
      hsize_t dims_fam[2];
      hid_t status_fam           = H5Sget_simple_extent_dims(filespace_fam, dims_fam, NULL);
      if(status_fam == 0) {
        std::cerr << "MED_IO::read dims not found";
        abort();
      }

      const unsigned n_elements = dims_fam[0];
//       std::vector< TYPE_FOR_INT_DATASET > fam_map(n_elements);
      fam_map.resize(n_elements);
      hid_t status_conn = H5Dread(dtset_fam,/*ONLY DIFFERENCE*/H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, fam_map.data());
      H5Dclose(dtset_fam);     
      
  }


  
 template < >  
  void MED_IO::dataset_open_and_close_store_in_vector<TYPE_FOR_REAL_DATASET>(hid_t file_id, std::vector< TYPE_FOR_REAL_DATASET > & fam_map, const std::string fam_name_dir_i) const  {
      
       hid_t dtset_fam            = H5Dopen(file_id, fam_name_dir_i.c_str(), H5P_DEFAULT);
      hid_t filespace_fam        = H5Dget_space(dtset_fam);
      hsize_t dims_fam[2];
      hid_t status_fam           = H5Sget_simple_extent_dims(filespace_fam, dims_fam, NULL);
      if(status_fam == 0) {
        std::cerr << "MED_IO::read dims not found";
        abort();
      }

      const unsigned n_elements = dims_fam[0];
//       std::vector< TYPE_FOR_REAL_DATASET > fam_map(n_elements);
      fam_map.resize(n_elements);
      hid_t status_conn = H5Dread(dtset_fam,/*ONLY DIFFERENCE*/ H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, fam_map.data());
      H5Dclose(dtset_fam);     
      
  }  
   
  

// MED_IO::~MED_IO() {
//     
//         for(unsigned g = 0; g < _geom_elems.size(); g++) delete _geom_elems[g];
// 
// }

// @todo hybrid meshes (MED can export them)
// @todo groups with more than one type of element (in the sense hybrid, but at the same dimension)
// @todo mesh with overlapping groups  (MED can export them, defining the intersection independently; now the name parsing wouldn't work with them; Salome splits into non-intersecting family, so one needs to read the GRO/NOM dataset )
// @todo how would I use a 0d element? or a ball?
// @todo triggering of the copy of new input files happens right after "Configure" of cmake
// @todo Redirect CERR together with COUT
// @todo Update Xdmf to Xdmf3
// @todo not running in parallel
// @todo I know I can make only groups of edges/faces/volumes, but can I do hybrid faces, say quad and tri together?
// @todo Generate MED format from other software such as Gmsh
// @todo FEHex20, FEQuad8, FETri7 missing

} // namespace femus

#endif 

#endif

