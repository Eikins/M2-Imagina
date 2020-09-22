#include <array>

#include "Octree.h"

Engine::Utils::OctreeNode::OctreeNode(const Engine::Bounds &bounds, unsigned maxItemPerNode)
{
	_bounds = bounds;
	_items.reserve(maxItemPerNode);
	_maxItems = maxItemPerNode;
	for (int i = 0; i < 8; i++)
	{
		_children[i] = nullptr;
	}
}

void Engine::Utils::OctreeNode::CreateChildrenAndMoveItems()
{
	if (_children[0] != nullptr) return;

	std::array<glm::vec3, 27> positions;
	std::array<Bounds, 8> bounds;
	// 0 -- 1 -- 2 .. 11 .. 20
	// |    |    |    |     |
	// 3 -- 4 -- 5 .. 14 .. 23
	// |    |    |    |     |
	// 6 -- 7 -- 8 .. 17 .. 26
	glm::vec3 diag = 0.5f * (_bounds.to - _bounds.from);
	glm::vec3 diagX = glm::vec3(diag.x, 0, 0);
	glm::vec3 diagY = glm::vec3(0, diag.y, 0);
	glm::vec3 diagZ = glm::vec3(0, 0, diag.z);

	// TODO : condense in for loop and remove unused positions
	positions[0] = _bounds.from;
	positions[1] = positions[0] + diagX;
	positions[2] = positions[1] + diagX;
	positions[3] = positions[0] + diagY;
	positions[4] = positions[3] + diagX;
	positions[5] = positions[4] + diagX;
	positions[6] = positions[3] + diagY;
	positions[7] = positions[6] + diagX;
	positions[8] = positions[7] + diagX;

	positions[9] = positions[0] + diagZ;
	positions[10] = positions[9] + diagX;
	positions[11] = positions[10] + diagX;
	positions[12] = positions[9] + diagY;
	positions[13] = positions[12] + diagX;
	positions[14] = positions[13] + diagX;
	positions[15] = positions[12] + diagY;
	positions[16] = positions[15] + diagX;
	positions[17] = positions[16] + diagX;

	positions[18] = positions[9] + diagZ;
	positions[19] = positions[18] + diagX;
	positions[20] = positions[19] + diagX;
	positions[21] = positions[18] + diagY;
	positions[22] = positions[21] + diagX;
	positions[23] = positions[22] + diagX;
	positions[24] = positions[21] + diagY;
	positions[25] = positions[24] + diagX;
	positions[26] = positions[25] + diagX;

	bounds[0].from = positions[0];
	bounds[0].to   = positions[13];
	bounds[1].from = positions[1];
	bounds[1].to   = positions[14];
	bounds[2].from = positions[3];
	bounds[2].to   = positions[16];
	bounds[3].from = positions[4];
	bounds[3].to   = positions[17];

	bounds[4].from = positions[9];
	bounds[4].to   = positions[22];
	bounds[5].from = positions[10];
	bounds[5].to   = positions[23];
	bounds[6].from = positions[12];
	bounds[6].to   = positions[25];
	bounds[7].from = positions[13];
	bounds[7].to   = positions[26];

	for (int i = 0; i < 8; i++)
	{
		https://stackoverflow.com/questions/33905030/how-to-make-stdmake-unique-a-friend-of-my-class
		_children[i] = std::unique_ptr<OctreeNode>(new OctreeNode(bounds[i], _maxItems));
	}

	for (int i = 0; i < _items.size(); i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (bounds[j].Contains(_items[i].position))
			{
				_children[j]->Insert(_items[i].position, _items[i]);
			}
		}
	}

	_items.clear();
}

bool Engine::Utils::OctreeNode::Insert(const glm::vec3 &position, const OctreeItem& item)
{
	if (_bounds.Contains(position) == false)
	{
		return false;
	}

	if (_children[0] != nullptr)
	{
		for (int i = 0; i < 8; i++)
		{
			if (_children[i]->Insert(position, item))
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		if (_items.size() >= _maxItems)
		{
			CreateChildrenAndMoveItems();

			for (int i = 0; i < 8; i++)
			{
				if (_children[i]->Insert(position, item))
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			_items.push_back(item);
			return true;
		}
	}
}

void Engine::Utils::OctreeNode::ApplyOnLeaves(const std::function<void(std::vector<OctreeItem> &items)> &func)
{
	if (_children[0] == nullptr)
	{
		if (_items.empty() == false)
		{
			func(_items);
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			_children[i]->ApplyOnLeaves(func);
		}
	}
}


Engine::Utils::Octree::Octree(const Bounds &bounds, int maxItemPerLeaf)
{
	_maxItemPerLeaf = maxItemPerLeaf;
https://stackoverflow.com/questions/33905030/how-to-make-stdmake-unique-a-friend-of-my-class
	// Can't use make_unique here
	_root = std::unique_ptr<OctreeNode>(new OctreeNode(bounds, maxItemPerLeaf));
}

bool Engine::Utils::Octree::Insert(const glm::vec3 &position, const OctreeItem &item)
{
	return _root->Insert(position, item);
}

void Engine::Utils::Octree::ApplyOnLeaves(const std::function<void(std::vector<OctreeItem> &items)> &func)
{
	_root->ApplyOnLeaves(func);
}