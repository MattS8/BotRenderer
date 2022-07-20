#include "bvh.h"

namespace end
{
	const aabb_t encapsulate(const aabb_t& first, const aabb_t& second)
	{
		float3 first_max = first.center + first.extents;
		float3 first_min = first.center - first.extents;
		float3 second_max = second.center + second.extents;
		float3 second_min = second.center - second.extents;

		float3 new_max = {
			std::max(first_max.x, second_max.x),
			std::max(first_max.y, second_max.y),
			std::max(first_max.z, second_max.z)
		};
		float3 new_min = {
			std::max(first_min.x, second_min.x),
			std::max(first_min.y, second_min.y),
			std::max(first_min.z, second_min.z)
		};


		float3 new_extents = (new_max - new_min) / 2;
		float3 new_center = new_max - new_extents;

		return { new_center, new_extents };
	}

	bvh_node_t::bvh_node_t(bvh_node_t* root, uint32_t left_index, uint32_t right_index)
	{
		//TODO The root pointer is the array of the entire bvh
		//Build the aabb that encapsulates the left and right's aabb's
	}

	float bounding_volume_hierarchy_t::cost(const bvh_node_t& a, const bvh_node_t& b)
	{
		//TODO: calculate manhattan distance from the center points of the two aabb's
		const aabb_t& a_aabb = a.aabb();
		const aabb_t& b_aabb = b.aabb();
		float3 cost = a_aabb.center - b_aabb.center;

		return abs(cost.x) + abs(cost.y) + abs(cost.z);
	}

	void bounding_volume_hierarchy_t::insert(const aabb_t& aabb, uint32_t element_id)
	{
		//TODO
		//create a bvh node using the passed in parameters(do not call new)
		bvh_node_t new_node(aabb, element_id);

		//TODO
		//if its the first node, it becomes the root. So just push it into bvh vector, then return
		if (bvh.size() == 0)
		{
			bvh.push_back(new_node);
			return;
		}

		//TODO
		//start at index 0 of the bvh (root)
		//while the current node is a branch, 
		//Modify this node's aabb that encapsulates the current node, and the aabb that was passed in
		//figure out if you are going with the left or right child
		//change your index to be the one you have chosen.
		bvh_node_t current_node = bvh[0];

		while (current_node.is_branch())
		{
			aabb_t new_aabb = encapsulate(current_node.aabb(), new_node.aabb());
			current_node.aabb().center = new_aabb.center;
			current_node.aabb().extents = new_aabb.extents;

			float l_cost = cost(bvh[current_node.left()], new_node);
			float r_cost = cost(bvh[current_node.right()], new_node);

			current_node = l_cost < r_cost
				? bvh[current_node.left()]
				: bvh[current_node.right()];
		}

		//TODO
		//Once you have left that while loop, you now hold onto a leaf node index where we will add the node to
		//the 2 new nodes' indices(left and right child) will be the newly created indices once you push back twice on the bvh vector
		//the current node (who is now a branch) needs to be sized to fit the two new nodes(left and right, remember don't need to call new).
		//set the parents of these two nodes
	}
}