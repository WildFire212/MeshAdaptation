#pragma once
#include"MeshLoader/Model.h"
#include<Eigen/Dense>
#include<glm.hpp>

struct RBFValues
{
	Eigen::VectorXf m_Lamdas;
	Eigen::MatrixXf m_PhiMatrixInverse;
	unsigned int m_PhiMatrixSize;

	void calculatePhiMatrix(std::vector<glm::vec3> controlPointPositions);
	float phi(float squaredDisplacements);
	float calculateInterpolantValue(glm::vec3 oldPos, std::vector<glm::vec3> controlPoints);
	void updateLambdas(std::vector<float> displacementArray);
};

class MeshAdapter
{
private: 
	const int m_NumControlPoints = 31 ; 
	float m_MorphTime = 0.0f;

	Model* m_SrcMesh; 
	Model* m_SrcMeshDeformed; 
	Model* m_DstMesh; 
	
	RBFValues xRBF, yRBF, zRBF;
	std::vector<glm::vec3> m_ControlPointPositions;	

	std::vector<glm::vec3> m_DestinationPointPositions;

	std::vector<float> m_ControlPointDisplacementX;	
	std::vector<float> m_ControlPointDisplacementY;	
	std::vector<float> m_ControlPointDisplacementZ;	
public: 
	MeshAdapter(Model* srcModel,Model* srcModelDeformed, Model* destModel, const int srcControlPoints[24], const int destControlPoints[24]);
	~MeshAdapter(); 
	void update(); 
	void deformMesh(); 
};

