cpu-style_SIMD_Ray_Traversal_on_GPUs.pdf
//int i = node_index * 4;
//nodes [ i +0] = vec4 ( box1.min.x , box1.max.x , box1.min.y , box1.max.y );
//nodes [ i +1] = vec4 ( box2.min.x , box2.max.x , box2.min.y , box2.max.y );
//nodes [ i +2] = vec4 ( box1.min.z , box1.max.z , box2.min.z , box2.max.z );
//nodes [ i +3] = vec4 ( child1.idx , child2.idx , 0 , 0 );

/*
*The first half of a node always consists of the x and y components of
its bounding volumes, followed by the second half containing their
corresponding z components and child indices.
*/
int i = node_index * 4;
nodes [ i +0] = vec4 ( box1.min.x , box1.max.x , box1.min.y , box1.max.y );
nodes [ i +1] = vec4 ( box2.min.x , box2.max.x , box2.min.y , box2.max.y );
nodes [ i +2] = vec4 ( box1.min.z , box1.max.z , child1.idx , 0 );
nodes [ i +3] = vec4 ( box2.min.z , box2.max.z , child2.idx , 0 );


//The thread group is trivially computed by
//dividing the lane index by the current group size (the local thread
//offset within the group is similarly obtained):
uint lane_group = lane / 4; // for groups of 4 threads
uint lane_offset = lane % 4; // for groups of 4 threads

//Again, the mask is trivially computed
//with the lane index (e.g for groups of 4):
uint group_mask = 0 x0000000f << ( lane & 0 xfffffffc );


if ( lane == 0)
ray_idx = atomicAdd ( g_warpCounter , num_groups );
ray_idx = __shfl ( ray_idx , 0) + my_group ;




int curr_idx = stack [ lane_group ][ stack_pointer - -];
vec4 xy = nodes [ curr_idx + lane_offset ];
vec4 zi = nodes [ curr_idx + 2 + lane_offset ];

