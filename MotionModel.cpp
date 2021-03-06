/*
 * MotionModel.h
 * Created: July 26, 2017
 * Author: Stephen Mylabathula
 */

#include "MotionModel.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <iostream>


MotionModel::MotionModel() {
  frame_length = 0.0f;
  name = "motion model";
  // instantiate the skeleton structure object
  skeleton_structure = SkeletonStructure();
}

MotionModel::~MotionModel() {
  // destroy
}

/*
 * Steps:
 * 1. Load and parse the text file.
 * 2. Initialize the frame length member.
 * 3. Initialize the skeleton structure member.
 * 4. Initialize the channels matrix.
 * 5. Initialize the motion phases.
 */
void MotionModel::load(std::string file_path) {
  // Load the text file containing JSON data.
  std::ifstream t(file_path.c_str());
  std::string json_data((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  // Parse the text file into a JSON document.
  rapidjson::Document json_document;
  json_document.Parse(json_data.c_str());
  assert(json_document.IsObject());   // ensure file was parsed correctly

  // TODO: We can add checks here to make sure all expected data is contained
  //       in the JSON document.

  // Process the frame length value.
  const rapidjson::Value& json_frame_length = json_document["frame_length"];
  // Set frame length member variable from JSON.
  assert(json_frame_length.IsFloat());   // ensure the type of frame length
  this->frame_length = json_frame_length.GetFloat();

  // Process the skeleton structure.
  const rapidjson::Value& json_skeleton_structure = json_document["skeleton_structure"];
  assert(json_skeleton_structure.IsObject());
  // Set skeleton structure length
  assert(json_skeleton_structure["length"].IsInt());
  this->skeleton_structure.length = json_skeleton_structure["length"].GetInt();
  // Set skeleton structure mass
  assert(json_skeleton_structure["mass"].IsInt());
  this->skeleton_structure.mass = json_skeleton_structure["mass"].GetInt();
  // Set skeleton structure angle type
  assert(json_skeleton_structure["angle"].IsString());
  this->skeleton_structure.angle = json_skeleton_structure["angle"].GetString();
  // Set skeleton structure type
  assert(json_skeleton_structure["type"].IsString());
  this->skeleton_structure.type = json_skeleton_structure["type"].GetString();
  // Set skeleton structure documentation
  assert(json_skeleton_structure["documentation"].IsString());
  this->skeleton_structure.documentation = json_skeleton_structure["documentation"].GetString();
  // Set skeleton structure name
  assert(json_skeleton_structure["name"].IsString());
  this->skeleton_structure.name = json_skeleton_structure["name"].GetString();
  // Set skeleton structure model height
  assert(json_skeleton_structure["model_height"].IsFloat());
  this->skeleton_structure.model_height = json_skeleton_structure["model_height"].GetFloat();
  // Set skeleton structure head index
  assert(json_skeleton_structure["head_index"].IsInt());
  this->skeleton_structure.head_index = json_skeleton_structure["head_index"].GetInt() - 1; // subtract one for zero basis
  // Set skeleton structure left foot index
  assert(json_skeleton_structure["left_foot_index"].IsInt());
  this->skeleton_structure.left_foot_index = json_skeleton_structure["left_foot_index"].GetInt() - 1; // subtract one for zero basis
  // Set skeleton structure right foot index
  assert(json_skeleton_structure["right_foot_index"].IsInt());
  this->skeleton_structure.right_foot_index = json_skeleton_structure["right_foot_index"].GetInt() - 1; // subtract one for zero basis


  // Set skeleton structure tree
  assert(json_skeleton_structure["tree"].IsArray());
  const rapidjson::Value& json_tree_array = json_skeleton_structure["tree"];
  for (rapidjson::Value::ConstValueIterator itr = json_tree_array.Begin(); itr != json_tree_array.End(); ++itr){
    JointNode current_joint_node;
    // set joint name
    current_joint_node.name = itr->GetObject()["name"].GetString();
    // set joint id
    current_joint_node.id = itr->GetObject()["id"].GetInt();
    // set joint offset
    current_joint_node.offset[0] = itr->GetObject()["offset"][0].GetFloat();
    current_joint_node.offset[1] = itr->GetObject()["offset"][1].GetFloat();
    current_joint_node.offset[2] = itr->GetObject()["offset"][2].GetFloat();
    // set joint orientation
    for (rapidjson::Value::ConstValueIterator itr_orientation = itr->GetObject()["orientation"].Begin(); itr_orientation != itr->GetObject()["orientation"].End(); ++itr_orientation){
      current_joint_node.orientation.push_back(itr_orientation->GetFloat());
    }
    // set joint axis
    current_joint_node.axis[0] = itr->GetObject()["axis"][0].GetFloat();
    current_joint_node.axis[1] = itr->GetObject()["axis"][1].GetFloat();
    current_joint_node.axis[2] = itr->GetObject()["axis"][2].GetFloat();
    // set joint axis order
    current_joint_node.axis_order = itr->GetObject()["axisOrder"].GetString();

    // set C and Cinv matrices
    for (int i = 0; i < 3; i++){
      for (int j = 0; j < 3; j++){
        current_joint_node.c(i, j) = itr->GetObject()["C"][i][j].GetFloat();
        current_joint_node.c_inv(i, j) = itr->GetObject()["Cinv"][i][j].GetFloat();
      }
    }

    // set joint channels
    for (rapidjson::Value::ConstValueIterator itr_channels = itr->GetObject()["channels"].Begin(); itr_channels != itr->GetObject()["channels"].End(); ++itr_channels){
      current_joint_node.channels.push_back(itr_channels->GetString());
    }
    // set joint bodymass
    if (itr->GetObject()["bodymass"].IsFloat())
      current_joint_node.bodymass = itr->GetObject()["bodymass"].GetFloat();
    else
      current_joint_node.bodymass = 0.0;
    // set joint confmass
    if (itr->GetObject()["confmass"].IsFloat())
      current_joint_node.confmass = itr->GetObject()["confmass"].GetFloat();
    else
      current_joint_node.confmass = 0.0;
    // set joint parent
    current_joint_node.parent = itr->GetObject()["parent"].GetInt() - 1;  // subtract one for zero basis
    // set joint order
    if (itr->GetObject()["order"].IsString())
      current_joint_node.order = itr->GetObject()["order"].GetString();
    else
      current_joint_node.order = "";
    // set joint rotind
    current_joint_node.rotind[0] = itr->GetObject()["rotInd"][0].GetInt() - 1;  // subtract one for zero basis
    current_joint_node.rotind[1] = itr->GetObject()["rotInd"][1].GetInt() - 1;  // subtract one for zero basis
    current_joint_node.rotind[2] = itr->GetObject()["rotInd"][2].GetInt() - 1;  // subtract one for zero basis
    // set joint posind
    current_joint_node.posind[0] = itr->GetObject()["posInd"][0].GetInt() - 1;  // subtract one for zero basis
    current_joint_node.posind[1] = itr->GetObject()["posInd"][1].GetInt() - 1;  // subtract one for zero basis
    current_joint_node.posind[2] = itr->GetObject()["posInd"][2].GetInt() - 1;  // subtract one for zero basis
    // set joint children
    if (itr->GetObject()["children"].IsArray())
      for (rapidjson::Value::ConstValueIterator itr_children = itr->GetObject()["children"].Begin(); itr_children != itr->GetObject()["children"].End(); ++itr_children){
        current_joint_node.children.push_back(itr_children->GetInt() - 1); // subtract one for zero basis
      }
    else
      current_joint_node.children.push_back(itr->GetObject()["children"].GetInt() - 1); // subtract one for zero basis
    // set joint limits
    std::vector<float> temp_for_rows;
    for (rapidjson::Value::ConstValueIterator itr_rows = itr->GetObject()["limits"].Begin(); itr_rows != itr->GetObject()["limits"].End(); ++itr_rows){
      if (itr_rows->IsArray()){
        for (rapidjson::Value::ConstValueIterator itr_columns = itr_rows->Begin(); itr_columns != itr_rows->End(); ++itr_columns){
          temp_for_rows.push_back(itr_columns->GetFloat());   // push each element in current row into temp vector
        }
      }
      else{
        temp_for_rows.push_back(itr_rows->GetFloat());
      }
      current_joint_node.limits.push_back(temp_for_rows);    // push this row into the 2D channels vector
      temp_for_rows.clear();
    }

    skeleton_structure.tree.push_back(current_joint_node);  // finally add this joint node to the tree
  }

  // Process the channels matrix
  const rapidjson::Value& json_channels = json_document["channels"];
  assert(json_channels.IsArray());
  std::vector<float> temp_for_rows;
  for (rapidjson::Value::ConstValueIterator itr_rows = json_channels.Begin(); itr_rows != json_channels.End(); ++itr_rows){
    for (rapidjson::Value::ConstValueIterator itr_columns = itr_rows->Begin(); itr_columns != itr_rows->End(); ++itr_columns){
      temp_for_rows.push_back(itr_columns->GetFloat());   // push each element in current row into temp vector
    }
    this->channels.push_back(temp_for_rows);    // push this row into the 2D channels vector
    temp_for_rows.clear();
  }

  // Process the motion phases structure.
  const rapidjson::Value& json_phases = json_document["phases"];
  assert(json_phases.IsObject());
  // Set motion phases start index
  assert(json_phases["start_index"].IsInt());
  this->motion_phases.start_index = json_phases["start_index"].GetInt() - 1; // subtract one for zero basis
  // Set motion phases end index
  assert(json_phases["end_index"].IsInt());
  this->motion_phases.end_index = json_phases["end_index"].GetInt() - 1; // subtract one for zero basis
  // Fill in the titles vector
  assert(json_phases["titles"].IsArray());
  const rapidjson::Value& json_titles_array = json_phases["titles"];
  for (rapidjson::Value::ConstValueIterator itr = json_titles_array.Begin(); itr != json_titles_array.End(); ++itr){
    motion_phases.titles.push_back(itr->GetString());
  }
  // Fill in the indices vector
  assert(json_phases["indices"].IsArray());
  const rapidjson::Value& json_indices_array = json_phases["indices"];
  for (rapidjson::Value::ConstValueIterator itr = json_indices_array.Begin(); itr != json_indices_array.End(); ++itr){
    motion_phases.indices.push_back(itr->GetInt() - 1); // subtract one for zero basis
  }
}

void MotionModel::print(){
  std::cout << "XYZ: " << std::endl;
  std::vector< std::vector<float> > xyz = this->skeleton_structure.ComputeSkeletonXYZPose(this->channels);
  for (unsigned int i = 0; i < xyz.size(); i++){
    for (unsigned int j = 0; j < 3; j++){
      std::cout << xyz[i][j] << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Frame Length: " << this->frame_length << std::endl << std::endl;
  std::cout << "Skeleton Structure: " << std::endl;
  std::cout << "   - Length: " << this->skeleton_structure.length << std::endl;
  std::cout << "   - Mass: " << this->skeleton_structure.mass << std::endl;
  std::cout << "   - Angle: " << this->skeleton_structure.angle << std::endl;
  std::cout << "   - Type: " << this->skeleton_structure.type << std::endl;
  std::cout << "   - Model Height: " << this->skeleton_structure.model_height << std::endl;
  std::cout << "   - Head Index: " << this->skeleton_structure.head_index << std::endl;
  std::cout << "   - Left Foot Index: " << this->skeleton_structure.left_foot_index << std::endl;
  std::cout << "   - Right Foot Index: " << this->skeleton_structure.right_foot_index << std::endl;
  std::cout << "   - Tree: " << std::endl;
  std::cout << "         - Size: " << this->skeleton_structure.tree.size() << std::endl;
  std::cout << "         - Names: [ ";
  for (unsigned int i = 0; i < this->skeleton_structure.tree.size(); i++)
    std::cout << this->skeleton_structure.tree[i].name << " ";
  std::cout << "]" << std::endl;
  std::cout << "         - C @ 18: " << std::endl << this->skeleton_structure.tree[18].c << std::endl;
  std::cout << "         - Children Sizes: [ ";
  for (unsigned int i = 0; i < this->skeleton_structure.tree.size(); i++)
    std::cout << this->skeleton_structure.tree[i].children.size() << " ";
  std::cout << "]" << std::endl;
  std::cout << "         - Limits Sizes: [ ";
  for (unsigned int i = 0; i < this->skeleton_structure.tree.size(); i++)
    std::cout << this->skeleton_structure.tree[i].limits.size() << " ";
  std::cout << "]" << std::endl << std::endl;
  std::cout << "Channels: " << std::endl;
  std::cout << "   - Size: " << this->channels.size() << std::endl;
  std::cout << "   - Element (51, 14): " << this->channels[51][14] << std::endl << std::endl;

  std::cout << "Phases: " << std::endl;
  std::cout << "   - Start Index: " << this->motion_phases.start_index << std::endl;
  std::cout << "   - End Index: " << this->motion_phases.end_index << std::endl;
  std::cout << "   - Titles: [ ";
  for (unsigned int i = 0; i < this->motion_phases.titles.size(); i++)
    std::cout << this->motion_phases.titles[i] << " ";
  std::cout << "]" << std::endl;
  std::cout << "   - Indices: [ ";
  for (unsigned int i = 0; i < this->motion_phases.indices.size(); i++)
    std::cout << this->motion_phases.indices[i] << " ";
  std::cout << "]" << std::endl;

}

