#pragma once

#include "ASTNode.hpp"

class AutoIndexNode : public ASTNode
{
	private:
    	bool _enabled;

	public:
    	AutoIndexNode(bool enabled);
    	bool getEnabled() const;
    	void resolve(const ASTNode& server);
};