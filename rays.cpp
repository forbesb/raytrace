#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "ray.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "hittable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float randfloat(){
    return ((float) rand() / RAND_MAX);
}

vec3 color(const ray& r, hittable *world, int depth) {
    hit_record rec;
    if(world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat->scatter(r, rec, attenuation, scattered)) {
            return attenuation*color(scattered, world, depth+1);
        } else {
            return vec3(0,0,0);
        }
    }
    vec3 unit_direction = unit_vector(r.direction());
    float t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

hittable* random_scene(int n) {
    hittable **list = new hittable*[n+1];
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(new checker_texture(
        new constant_texture(vec3(0.2,0.3,0.1)), 
        new constant_texture(vec3(0.9,0.9,0.9)))));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = randfloat();
            vec3 center(a+0.9*randfloat(), 0.2, b+0.9*randfloat());
            if ((center - vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {
                    list[i++] = new moving_sphere(center, center+vec3(0,0.1+randfloat(), 0), 0.0, 1.0, 0.2, new lambertian(new constant_texture(vec3(randfloat() * randfloat(), randfloat() * randfloat(), randfloat() * randfloat()))));
                } else if (choose_mat < 0.95) {
                    list[i++] = new sphere(center, 0.2,
                        new metal(vec3(0.5*(1+randfloat()), 0.5*(1+randfloat()), 0.5*(1+randfloat())), 0.5*randfloat()));
                } else {
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0,1,0),1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4,1,0), 1.0, new lambertian(new constant_texture(vec3(0.4,0.2,0.1))));
    list[i++] = new sphere(vec3(4,1,0), 1.0, new metal(vec3(0.7,0.6,0.5),0.0));

    return new bvh_node(list, i, 0.0,1.0);
}

hittable* two_perlin_spheres() {
    texture *pertext = new noise_texture(2);
    hittable **list = new hittable*[2];
    list[0] = new sphere(vec3(0,-1000,0),1000,new lambertian(pertext));
    list[1] = new sphere(vec3(0,2,0),2,new lambertian(pertext));
    return new hittable_list(list,2);
}

hittable* image() {
    int nx, ny, nn;
    unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    material *mat = new lambertian(new image_texture(tex_data, nx, ny));
    return new sphere(vec3(0,0,0), 2, mat);
}


int main() {
    
    srand(time(NULL));

    int nx = 1200;
    int ny = 800;
    int ns = 10;
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    //hittable *world = random_scene(5000);
    //hittable *world = two_perlin_spheres();
    hittable *world = image();
    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float t0 = 0.0, t1=1.0;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus, t0, t1);

    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0,0,0);
            for (int s = 0; s < ns; s++) {
                float u = float(i+randfloat()) / float(nx);
                float v = float(j+randfloat()) / float(ny); 
                ray r = cam.get_ray(u, v);
                vec3 p = r.at(2.0);
                vec3 cc = color(r, world, 0);
                col += cc;
            }

            col /= float(ns);

            float r = sqrt(col.r());
            float g = sqrt(col.g());
            float b = sqrt(col.b());
            col = vec3( r, g, b );

            int ir = int(255.99 * col.r());
            int ig = int(255.99 * col.g());
            int ib = int(255.99 * col.b());
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
