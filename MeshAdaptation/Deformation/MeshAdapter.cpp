#include "MeshAdapter.h"


MeshAdapter::MeshAdapter(Model* srcModel,Model* srcModelDeformed, Model* destModel, const int srcControlPoints[31], const int destControlPoints[31]) :
	m_SrcMesh(srcModel),
	m_SrcMeshDeformed(srcModelDeformed), 
	m_DstMesh(destModel)
{
	for (unsigned int i = 0; i < m_NumControlPoints; i++)
	{
		m_ControlPointPositions.push_back(m_SrcMesh->getVertices()->at(srcControlPoints[i]).positions);																							
	}	
	
	for (unsigned int i = 0; i < m_ControlPointPositions.size(); i++)
	{
		m_ControlPointDisplacementX.push_back(0.0f);
		m_ControlPointDisplacementY.push_back(0.0f);
		m_ControlPointDisplacementZ.push_back(0.0f);
	}

	for (unsigned int i = 0; i < m_NumControlPoints; i++)
	{
		m_DestinationPointPositions.push_back(m_DstMesh->getVertices()->at(destControlPoints[i]).positions);
	}																					
	
	xRBF.calculatePhiMatrix(m_ControlPointPositions);
	yRBF.calculatePhiMatrix(m_ControlPointPositions);
	zRBF.calculatePhiMatrix(m_ControlPointPositions);


}

MeshAdapter::~MeshAdapter()
{
}

void MeshAdapter::update()
{
	
	m_MorphTime += 0.05f; 
	
	for (unsigned int i = 0; i < m_ControlPointPositions.size(); i++)
	{
		glm::vec3 displacements = (m_DestinationPointPositions[i] - m_ControlPointPositions[i]);
		
		displacements.x *=(-cosf(m_MorphTime) / 2 + 0.5f);;
		displacements.y *=(-cosf(m_MorphTime) / 2 + 0.5f);;
		displacements.z *=(-cosf(m_MorphTime) / 2 + 0.5f);;
		
		m_ControlPointDisplacementX[i] = displacements.x;
		m_ControlPointDisplacementY[i] = displacements.y;
		m_ControlPointDisplacementZ[i] = displacements.z;
	}

	xRBF.updateLambdas(m_ControlPointDisplacementX);
	yRBF.updateLambdas(m_ControlPointDisplacementY);
	zRBF.updateLambdas(m_ControlPointDisplacementZ);

	deformMesh(); 

}

void MeshAdapter::deformMesh()
{
	for (unsigned int  i = 0; i < m_SrcMeshDeformed->getVertices()->size(); i++)
	{
		glm::vec3 oldPos = m_SrcMesh->getVertices()->at(i).positions; 
		glm::vec3 newPos;
			
		newPos.x = oldPos.x + xRBF.calculateInterpolantValue(oldPos,m_ControlPointPositions);
		newPos.y = oldPos.y + yRBF.calculateInterpolantValue(oldPos,m_ControlPointPositions);
		newPos.z = oldPos.z + zRBF.calculateInterpolantValue(oldPos,m_ControlPointPositions);

		m_SrcMeshDeformed->getVertices()->at(i).positions = newPos;
	}
	m_SrcMeshDeformed->createMesh();
}

void RBFValues::calculatePhiMatrix(std::vector<glm::vec3> controlPointPositions)
{
	m_PhiMatrixSize = controlPointPositions.size();

	Eigen::MatrixXf phiMatrix(m_PhiMatrixSize + 4, m_PhiMatrixSize + 4);
	
	for (unsigned int i = 0; i < m_PhiMatrixSize; i++)
	{
		for (unsigned int j = 0; j < m_PhiMatrixSize; j++)
		{
			float dx = controlPointPositions[i].x - controlPointPositions[j].x;
			float dy = controlPointPositions[i].y - controlPointPositions[j].y;
			float dz = controlPointPositions[i].z - controlPointPositions[j].z;

			float squaredDisplacements = dx * dx + dy * dy + dz * dz;

			phiMatrix(i, j) = phi(squaredDisplacements);
		}
	}
	for (unsigned int i = 0; i < m_PhiMatrixSize; i++)
	{
		phiMatrix(i, m_PhiMatrixSize + 0) = 1;
		phiMatrix(i, m_PhiMatrixSize + 1) = controlPointPositions[i].x;
		phiMatrix(i, m_PhiMatrixSize + 2) = controlPointPositions[i].y;
		phiMatrix(i, m_PhiMatrixSize + 3) = controlPointPositions[i].z;
	}

	for (unsigned int j = 0; j < m_PhiMatrixSize; j++)
	{
		phiMatrix(m_PhiMatrixSize, j) = 1;
		phiMatrix(m_PhiMatrixSize + 1, j) = controlPointPositions[j].x;
		phiMatrix(m_PhiMatrixSize + 2, j) = controlPointPositions[j].y;
		phiMatrix(m_PhiMatrixSize + 3, j) = controlPointPositions[j].z;
	}

	for (unsigned int i = m_PhiMatrixSize; i < m_PhiMatrixSize + 4; i++)
		for (unsigned int j = m_PhiMatrixSize; j < m_PhiMatrixSize + 4; j++)
			phiMatrix(i, j) = 0;


	//Phi Matrix Calculation
	m_PhiMatrixInverse = phiMatrix.inverse();
}

float RBFValues::calculateInterpolantValue(glm::vec3 oldPos , std::vector<glm::vec3> controlPoints)
{
	float sum = 0.0f;	

	for (unsigned int i = 0; i < m_PhiMatrixSize; i++)
	{
		float dx = oldPos.x - controlPoints[i].x;
		float dy = oldPos.y - controlPoints[i].y;
		float dz = oldPos.z - controlPoints[i].z;
			

		float distanceSquared = dx * dx + dy * dy + dz * dz;
			
		sum += m_Lamdas(i) * phi(distanceSquared);
	}

	//Interpolated value between the src and dest using RBF
	sum +=	m_Lamdas(m_PhiMatrixSize + 0) +
			m_Lamdas(m_PhiMatrixSize + 1) * oldPos.x +
			m_Lamdas(m_PhiMatrixSize + 2) * oldPos.y +
			m_Lamdas(m_PhiMatrixSize + 3) * oldPos.z;

	return sum;
}

void RBFValues::updateLambdas(std::vector<float> displacementArray)
{
	Eigen::VectorXf Fis(m_PhiMatrixSize + 4);

	for (unsigned int i = 0; i < m_PhiMatrixSize; i++)
		Fis(i) = displacementArray[i];

	Fis(m_PhiMatrixSize + 0) = 0;
	Fis(m_PhiMatrixSize + 1) = 0;
	Fis(m_PhiMatrixSize + 2) = 0;
	Fis(m_PhiMatrixSize + 3) = 0;

	m_Lamdas = m_PhiMatrixInverse * Fis;
}

float RBFValues::phi(float squareDisplacements)
{
	return sqrt(log10(squareDisplacements + 1.0f));
}

	