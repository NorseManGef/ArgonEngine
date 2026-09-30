
// OpenCL FHN Neuron Simulation
// Skyler Saleh

// In this kernel 4 neurons are integrated simultaneously per thread to utilize all the
// lanes in the GPU's SIMD units.

// TODO:
// -Allow for cross neuron communication
// -Pull all removeable constants out of the integration, and calculate them on the cpu
inline float4 back_color(float4 position,float4 ray_dir){
   position+=ray_dir*1.0f;
   
   
   if(((int)(position[1]*40.f+position[0]*20.f)%2))return (float4)(0.2f);
   else return (float4)(0.0f);
}
void get_closest_object(float4 ray_start, float4 dir, float *dist, int *closest,  __global float4* objects, int object_count){
   for(int i=0;i<object_count;++i){
      float4 o = objects[i];
      float4 ray_p=ray_start-o;
      ray_p.s3=0.f;
      const float s2 = dot(dir,dir);
      const float s1 = dot(ray_p, dir);
      const float s0 = o.s3*o.s3-dot(ray_p,ray_p);

      float d = s1 * s1 + s2 * s0;
      if(d>0.001f){
         d=(s1 + sqrt(d))/(-s2);
         if(d>0.001f&& d<*dist ){
         *closest=i;
         *dist =d;
         }
      }
      
   }
}
__kernel void get_ray_object(__global float4* ray_start, __global float4* ray_dir, __global float* dist, __global int* closest, int object_id, float4 object){
   int i = get_global_id(0);
   
   float4 o = object;
   float4 ray_p=ray_start[i]-o;
   float4 dir=ray_dir[i];
   dir.s3=0.f;
   ray_p.s3=0.f;
   const float s2 = dot(dir,dir);
   const float s1 = dot(ray_p, dir);
   const float s0 = o.s3*o.s3-dot(ray_p,ray_p);
   
   float d = s1 * s1 + s2 * s0;
   if(d>0.001f){
      d=(s1 + sqrt(d))/(-s2);
      if(d>0.001f&& d<dist[i] ){
         closest[i]=object_id;
         dist[i] =d;
      }
   }
   
}
float4 get_shadow_factor(float4 ray_start, float4 dir, float dist, __global float4* objects, __global float4* object_colors, __global float4* object_props, int object_count){
   
   float4 shadow_factor=(float4)(1.f);
   for(int i=0;i<object_count;++i){
      float4 o = objects[i];
      float4 ray_p=ray_start-o;
      ray_p.s3=0.f;
      const float s2 = dot(dir,dir);
      const float s1 = dot(ray_p, dir);
      const float s0 = o.s3*o.s3-dot(ray_p,ray_p);
      
      float d = s1 * s1 + s2 * s0;
      if(d>0.001f){
         d=(s1 + sqrt(d))/(-s2);
         if(d>0.001f&& d<dist ){
            float t = object_props[i].s3;
            shadow_factor*=t*object_colors[i];
         }
      }
      
   }
   return shadow_factor;
}
inline float4 calc_lighting(float spec_ratio,float4 normal,float4 ray_pos, float4 ray_dir, __global float4 *light_colors,
                            __global float4* light_positions, __global float4* objects,__global float4* object_colors, __global float4* object_props, int total_lights, int total_objects){
   float4 color=(float4)(0.1f);
   float4 spec_color=(float4)(0.1f);
   ray_dir.s3=ray_pos.s3=normal.s3=0.f;
   for(int i=0;i<total_lights;++i){
      float4 light_dist=light_positions[i]-ray_pos;
      light_dist.s3=0.0f;

      float dist = length(light_dist);
      float4 light_dir =light_dist/dist;
      float4 shadow_factor = get_shadow_factor(ray_pos, light_dir,  dist, objects,object_colors,object_props,total_objects);
      
      if(dot(light_dir,normal) >0.f){
         float4 light_factor = light_colors[i]*shadow_factor;
         
         float lambert = dot(light_dir,normal)*(1.0f-spec_ratio);
         
         color+=light_factor*lambert;
         float4 blinn_dir = normalize(light_dir-ray_dir);
         float blinn_term = max(dot(blinn_dir,normal),0.f);
         spec_color+=light_factor*pow(blinn_term, 30.f);
         
         
      }
   }
   return color+spec_color*spec_ratio;
   
}
inline float4 refract(float4 incident, float4 normal, float eta){
   incident*=eta;
   float ni = dot(normal, incident);
   float k = 1.f-eta*eta+ni*ni;
   return k<0.0f? (float4)(0.f): incident-(ni+sqrt(k))*normal;
}
float4 do_ray2(float4 ray_start,float4 ray_dir,
              __global float4* light_colors, __global float4* light_positions, int total_lights,
              __global float4* objects,  __global float4* object_colors, __global float4* object_props,int object_count,int depth){
   float dist = 1e8f;
   int closest;
   get_closest_object(ray_start,ray_dir,&dist,&closest,objects,object_count);
   if(dist!=1.e8f&&depth<3){
      float4 color = object_colors[closest];
      
      float4 intersect_point = ray_start+ray_dir*dist;
      intersect_point.s3=0.0f;
      float4 ob = objects[closest];
      float4 p = (float4)(ob.s0,ob.s1,ob.s2,0.f);
      float4 n = normalize(intersect_point-p);
      
      float index_of_ref = object_props[closest].s0;
      float transparency =object_props[closest].s3;
      float fresnal_factor=(index_of_ref-1.0f)/(1.0f+index_of_ref);
      fresnal_factor*=fresnal_factor;
      float ratio   = fresnal_factor + (1.0f - fresnal_factor) * pow((1.0f - dot(-1.0f*ray_dir, n)), 5.0f);
      float4 light_c = calc_lighting(ratio,n,intersect_point,ray_dir,light_colors,light_positions,objects,object_colors,object_props,total_lights,object_count);
      
      return light_c;
      
   }else return back_color(ray_start,ray_dir);
   
}

float4 do_ray(float4 ray_start,float4 ray_dir,
              __global float4* light_colors, __global float4* light_positions, int total_lights,
              __global float4* objects,  __global float4* object_colors, __global float4* object_props,int object_count,int depth){
   float dist = 1e8f;
   int closest;
   get_closest_object(ray_start,ray_dir,&dist,&closest,objects,object_count);
   if(dist!=1.e8f&&depth<3){
      float4 color = object_colors[closest];

      float4 intersect_point = ray_start+ray_dir*dist;
      intersect_point.s3=0.0f;
      float4 ob = objects[closest];
      float4 p = (float4)(ob.s0,ob.s1,ob.s2,0.f);
      float4 n = normalize(intersect_point-p);
      
      float index_of_ref = object_props[closest].s0;
      float transparency =object_props[closest].s3;
      float fresnal_factor=(index_of_ref-1.0f)/(1.0f+index_of_ref);
      fresnal_factor*=fresnal_factor;
      float ratio   = fresnal_factor + (1.0f - fresnal_factor) * pow((1.0f - dot(-1.0f*ray_dir, n)), 5.0f);
      float4 light_c = calc_lighting(ratio,n,intersect_point,ray_dir,light_colors,light_positions,objects,object_colors,object_props,total_lights,object_count);

      float reflect = 2.0f * dot(ray_dir,n);
      float4 reflected=do_ray2(intersect_point, ray_dir-reflect*n, light_colors,light_positions,total_lights,objects,object_colors,object_props,object_count,depth+1);
      float4 ref= refract(normalize(ray_dir),n,1.0f/index_of_ref);
      float4 refracted=do_ray2(intersect_point, ref, light_colors,light_positions,total_lights,objects,object_colors,object_props,object_count,depth+1);
      
      return mix(color*(light_c+refracted)*(1.0f-transparency)+refracted*transparency,reflected,ratio);
      
   }else return back_color(ray_start,ray_dir);
   
}
__kernel void ray_trace(__global float4* ray_start,__global float4* ray_color,
                              __global float4* light_colors, __global float4* light_positions, int total_lights,
                              __global float4* objects, __global float4* object_colors, __global float4* object_props, int object_count )
{
   int i = get_global_id(0);
    
   ray_color[i]=do_ray(ray_start[i],(float4)(0.f,0.f,-1.f,0.f),light_colors,light_positions,total_lights,objects,object_colors,object_props,object_count,0);
   
}

