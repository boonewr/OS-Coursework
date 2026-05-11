#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

// Maximum length of a name.
#define NAME_MAX 23

// definition of a Point Of Interest, with a name and a location as
// GPS coordinates.
typedef struct POI
{
    // Name of this location.
    char name[NAME_MAX + 1];

    // GPS cordinates of this location.
    double lat, lon;
} POI;

static void fail(char const *msg)
{
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

/** Multiplier for converting degrees to radians */
#define DEG_TO_RAD (M_PI / 180)

/** Radius of the earth in miles. */
#define EARTH_RADIUS 3959.0

// Target location latitude and longitude.
double targetLat, targetLon;

/** Given the latitude and longitude of a locations, return the (approximate)
    distance to the target location, in miles. */
double globalDistance(double lat, double lon)
{
    // OK, pretend the center of the earth is at the origin, turn the
    // given location and the target location intointo vectors pointing
    // from the origin (this could be simplified).
    double v1[] = {cos(lon * DEG_TO_RAD) * cos(lat * DEG_TO_RAD),
                   sin(lon * DEG_TO_RAD) * cos(lat * DEG_TO_RAD),
                   sin(lat * DEG_TO_RAD)};

    double v2[] = {cos(targetLon * DEG_TO_RAD) * cos(targetLat * DEG_TO_RAD),
                   sin(targetLon * DEG_TO_RAD) * cos(targetLat * DEG_TO_RAD),
                   sin(targetLat * DEG_TO_RAD)};

    // Dot product these two vectors.
    double dp = 0.0;
    for (int i = 0; i < sizeof(v1) / sizeof(v1[0]); i++)
        dp += v1[i] * v2[i];

    // Compute the angle between the vectors based on the dot product.
    double angle = acos(dp);

    // Return the great circle distance based on the radius of the earth.
    return EARTH_RADIUS * angle;
}

int main(int argc, char *argv[])
{
    int fd;

    // Parse the command-line arguments and open the input file.
    if (argc != 4 ||
        (fd = open(argv[1], O_RDWR)) < 0 ||
        sscanf(argv[2], "%lf", &targetLat) != 1 ||
        sscanf(argv[3], "%lf", &targetLon) != 1)
        fail("usage: sort <file.dat> <latitude> <longitude>");

    // Use fstat to figure out the size of the file in bytes and the number of
    // POI structs that fit in the file.
    struct stat statbuf;
    if (fstat(fd, &statbuf) != 0)
        fail("Can't stat file");

    size_t fileSize = statbuf.st_size;
    size_t numPOIs = fileSize / sizeof(POI);

    // Use mmap to map the file into our logical address space.
    POI *plist = mmap(NULL,
                      fileSize,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fd,
                      0);

    if (plist == MAP_FAILED)
        fail("Can't map file file\n");

    // Sort the POI structs in the file based on their proximity to the given
    // target location, closer instances first.

    for (size_t i = 0; i < numPOIs - 1; i++)
    {
        for (size_t j = i + 1; j < numPOIs; j++)
        {
            double dist1 = globalDistance(plist[i].lat, plist[i].lon);
            double dist2 = globalDistance(plist[j].lat, plist[j].lon);
            if (dist2 < dist1)
            {
                POI tmp = plist[i];
                plist[i] = plist[j];
                plist[j] = tmp;
            }
        }
    }

    // Print out all the POI structs, now in sorted order with distance
    // to each location.

    for (size_t i = 0; i < numPOIs; i++)
    {
        double dist = globalDistance(plist[i].lat, plist[i].lon);
        printf("%23s %9.4f %9.4f (%.2f miles)\n",
               plist[i].name, plist[i].lat, plist[i].lon, dist);
    }

    // Use munmap() to un-map the file from memory.

    munmap(plist, fileSize);

    // Close the file.
    close(fd);

    // Terminate successfully.
    return EXIT_SUCCESS;
}
