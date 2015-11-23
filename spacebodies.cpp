// pvxf29

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>

#include <math.h>
#include <stdlib.h> // Rand()
/*
// Space Body initialisation values
#define BODY_COUNT 100           // Seed K Space Body instances
#define MAX_MASS 50             // Seed bodies with mass 0 to K
#define MAX_POS (int(BODY_COUNT)) // Seed bodies with position vectors of value 0 to K
#define MAX_VEL (int(2 * sqrt(MAX_MASS)))               // Seed bodies with velocity vectors of value 0 to K

// Simulation variables
#define COLL_DIST (int(MAX_MASS/100))           // Collision distance
#define TIME_STEPS 2000000      // No. steps
#define TIME_STEP 0.001         // Step size
#define PLOT_STEP 100           // Plot every K steps
*/

// Space Body initialisation values
int bodyCount = 100;                    // Seed K Space Body instances
int maxMass = 50;                       // Seed bodies with mass 0 to K
int maxPos = (int(bodyCount));         // Seed bodies with position vectors of value 0 to K
int maxVel = (int(2 * sqrt(maxMass)));  // Seed bodies with velocity vectors of value 0 to K

// Simulation variables
int collDist = (int(maxMass/100));      // Collision distance
int timeSteps = 2000000;                // No. steps
int timeStep = 0.001;                   // Step size
int plotStep = 100;                     // Plot every K steps

// -------------------- 3D vector struct and vector handling methods --------------------------- //

typedef struct {
	double x, y, z;
} Vec3;

Vec3 addVec( Vec3 a, Vec3 b ) {
	return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 subVec( Vec3 a, Vec3 b ) {
	return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 multVec( Vec3 a, double k ) {
	return (Vec3){ k * a.x, k * a.y, k * a.z };
}

inline double dotVec( Vec3 a, Vec3 b ) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline double magVec( Vec3 a ) {
	return sqrt( dotVec( a, a ) );
}

// -------------------- Space Body struct and methods --------------------------- //

typedef struct SpaceBody {
	double mass;
	Vec3 pos, vel, force;
	struct SpaceBody* collided; //Pointer to a body this body has collided with
} SpaceBody;

SpaceBody createSpaceBody( double mass, Vec3 pos, Vec3 vel ) {
    SpaceBody p;
    p.mass = mass;
    p.pos = pos;
    p.vel = vel;
    p.force = (Vec3){};
    p.collided = NULL;

    return p;
}

SpaceBody resolveCollision( SpaceBody a, SpaceBody b ) {
    SpaceBody p;
    p.mass = a.mass + b.mass;
    p.pos = multVec( addVec( a.pos, b.pos ), 0.5 ); // Position is halfway between the two bodies
    p.vel = multVec( addVec( multVec( a.vel, a.mass ), multVec( b.vel, b.mass ) ), 1.0 / p.mass);
    p.force = (Vec3){};

    return p;
}

std::vector<SpaceBody> spaceBodies; // Array of SpaceBodies
std::vector<std::pair<int, int>> collisionPairs; //Array of collision idex pairs

// -------------------- Main functions --------------------------- //

// Create CSV with space body positions at each timestep
void printCSVFile(int counter) {
	std::stringstream filename;
	filename << "output/result-" << counter <<  ".csv";
	std::ofstream out( filename.str().c_str() );

	out << "x, y, z" << std::endl;

	for ( int i = 0; i < bodyCount; i++ ) {
        out << spaceBodies[i].pos.x
            << ","
            << spaceBodies[i].pos.y
            << ","
            << spaceBodies[i].pos.z
            << std::endl;
    }
}

// Update all bodies
void updateBodies() {
    collisionPairs.clear();

    // Update forces
    #pragma omp parallel f
    for ( int i = 0; i < bodyCount; i++ ) {
        spaceBodies[i].force = (Vec3){};

        for ( int j = 0; j < bodyCount; j++ ) {
            // Ignore self and collided spaceBodies
            if ( (i != j) && (spaceBodies[j].collided == NULL) ) {
                double distance = magVec( subVec( spaceBodies[i].pos, spaceBodies[j].pos ) );

                spaceBodies[i].force = addVec(
                    spaceBodies[i].force, multVec(
                        subVec( spaceBodies[j].pos, spaceBodies[i].pos ),
                        spaceBodies[j].mass / (distance * distance * distance)
                    )
                );

                if ( (j > i) && (i < bodyCount) && (distance < collDist) && (spaceBodies[j].collided == NULL) ) {
                    // printf( "Collision: %d , %d\n", i, j );
                    collisionPairs.push_back({i,j});
                }
            }
        }
    }

    //Check for collisions
    for (int i = 0; i < collisionPairs.size(); i++) {
        spaceBodies[collisionPairs[i].first] = resolveCollision( spaceBodies[collisionPairs[i].first], spaceBodies[collisionPairs[i].second] );
        spaceBodies[collisionPairs[i].second].collided = &spaceBodies[collisionPairs[i].first];
    }

    // Calculate position and velocity
    #pragma omp parallel for
    for (int i = 0; i < bodyCount; i++) {
        if ( spaceBodies[i].collided != NULL ) {
            // In the event the particle is part of a collided pair
            spaceBodies[i].pos = (*spaceBodies[i].collided).pos;
        } else {
            // Otherwise calculate the vector stuff
            spaceBodies[i].pos = addVec( spaceBodies[i].pos, multVec( spaceBodies[i].vel, timeStep ) );
            spaceBodies[i].vel = addVec( spaceBodies[i].vel, multVec( spaceBodies[i].force, timeStep ) );
        }
    }
}

int main(int argc, char* argv) {
    // Simulation variables
    if (argv.size() >= 1){ int collDist = argv[1];}      // Collision distance
    if (argv.size() >= 2){ int timeSteps = argv[2];}                // No. steps
    if (argv.size() >= 3){ int timeStep = argv[3];}                   // Step size
    if (argv.size() >= 4){ int plotStep = argv[4];}                     // Plot every K steps
    // Space Body initialisation values
    if (argv.size() >= 5){ int bodyCount = argv[5];}                   // Seed K Space Body instances
    if (argv.size() >= 6){ int maxMass = argv[6];}                      // Seed bodies with mass 0 to K
    if (argv.size() >= 7){ int maxMass = argv[7];}        // Seed bodies with position vectors of value 0 to K
    if (argv.size() >= 8){ int MaxVel = argv[8];}  // Seed bodies with velocity vectors of value 0 to K

    //#pragma omp parallel for //Worth parallelising?
    for (int i=0; i < bodyCount; i++) { //Seed K many space bodies
        spaceBodies.push_back( 
            createSpaceBody(   (double)(rand() % maxMass ), //Body mass
                (Vec3){ //Body position
                    (double)( rand() % maxPos ),
                    (double)( rand() % maxPos ),
                    (double)( rand() % maxPos )}, 
                (Vec3){ //Body velocity
                    (double)( rand() % maxVel - (maxVel / 2) ),
                    (double)( rand() % maxVel - (maxVel / 2) ),
                    (double)( rand() % maxVel - (maxVel / 2) )
                    //0.0, 0.0, 0.0 //Start static
                } ) );
    }

    // Print starting pos 
	printCSVFile(0); // Please switch off all IO if you do performance tests.

	clock_t t1, t2;
	t1 = clock();

	for (int i=0; i < timeSteps; i++) {
		updateBodies();

		if (i % plotStep == 0) { //Check for plot
			printCSVFile(i / plotStep + 1); // Please switch off all IO if you do performance tests.
		}
	}

	t2 = clock();
	std::cout << "Simulation time: " << ((float)t2-(float)t1)/1000.0 << "ms" << std::endl; //Output time taken

	return 0;
}
