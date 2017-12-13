#pragma once
#include "IModel.h"

/** 
*	A plane model that is easy to render
*/
class PlaneModel : public IModel
{
public:
	// Methods
	PlaneModel(const std::string& sVSFileName, const std::string& sFSFileName, float side);

	void	render() const;

protected:
	// Fields
	float m_side;
};

