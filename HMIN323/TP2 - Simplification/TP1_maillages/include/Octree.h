#pragma once

#include <functional>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include "Bounds.h"

namespace Engine
{
	namespace Utils
	{
		// Todo: replace with template
		struct OctreeItem
		{
		public:
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			unsigned index;
		};

		class Octree;
		class OctreeNode;

		class Octree
		{
		private:
			unsigned _maxItemPerLeaf;
			std::unique_ptr<OctreeNode> _root;
		public:
			Octree(const Bounds &bounds, int maxItemPerLeaf = 8);
			bool Insert(const glm::vec3 &position, const OctreeItem &item);
			void ApplyOnLeaves(const std::function<void(std::vector<OctreeItem> &items)> &func);
		};

		class OctreeNode
		{
		friend class Octree;
		private:
			Bounds _bounds;
			std::vector<OctreeItem> _items;
			unsigned _maxItems;
			std::array<std::unique_ptr<OctreeNode>, 8> _children;

			OctreeNode(const Bounds& bounds, unsigned maxItemPerNode);
			void CreateChildrenAndMoveItems();
			bool Insert(const glm::vec3& position, const OctreeItem& item);
			void ApplyOnLeaves(const std::function<void(std::vector<OctreeItem>& items)>& func);
		};

	}
}