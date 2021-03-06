// Name        : star_clustering.cpp
// Author      : Shana Li, Jane Hsieh, Aidan KW
// Version     :
// Copyright   : Your copyright notice
// Description : K-Means clustering algorithm in C++
//============================================================================

#include "clustering.h"
#include "gnuplot-iostream.h"
#include <boost/lexical_cast.hpp>

using namespace std;

class Point {
private:
    double x;
    double y;
    string name;
public:
    bool clustered; // whether or not this point has a cluster yet
    Cluster *clust; // the cluster this point belongs in

    Point(double x, double y) {
        this->x = x;
        this->y = y;
        clustered = false;
    }

    string getName() {
        return this->name;
    }

    double getx() {
        return this->x;
    }

    double gety() {
        return this->y;
    }

    // calculate the distance between this point and another coordinate, based on the pythagorean theorem
    double dist(double destx, double desty) {
        double dx = (destx - this->x) * (destx - this->x);
        double dy = (desty - this->y) * (desty - this->y);
        return (sqrt(dx + dy));
    }

    // give the point a cluster
    void setClust(Cluster *clust) {
        clustered = true;
        this->clust = clust;
    }
};

class Cluster {
    // Class for a determined cluster of points
public:

    // collection of all the points in the cluster
    vector<Point*> points;

    // coordinates of cluster center
    double centerx;
    double centery;

    // average distance to center
    double avgdist;

    Cluster(double centerx, double centery) {
        this->centerx = centerx;
        this->centery = centery;
        avgdist = 0;
    }

    // returns coordinates of center as tuple
    tuple<double, double> getCoords() {
        return make_tuple(centerx, centery);
    }

    int numPoints() {
        return points.size();
    }

    // add a point to the vector of points, set parameters of the point
    void addPoint(Point *newPoint) {
        points.push_back(newPoint);
        newPoint->setClust(this);
    }

    // set the average distance of points to the center
    double setAvgDist() {
        double avgd = 0;
        for (const auto& a : points) {
            Point p = *a;
            avgd += p.dist(centerx, centery);
        }
        avgd = avgd / numPoints();
        avgdist = avgd;
        return avgd;
    }

    // remove a point
    void removePoint(Point *p) {
        // find its position in points
        vector<Point*>::iterator position = find(points.begin(), points.end(), p);

        // erase if found
        if (position != points.end()) {
            points.erase(position);
        }
    }

    // recenters the cluster, returns old center coordinate in tuple
    tuple<double, double> recenter() {

        // remember old center
        tuple<double, double> old = getCoords();

        // average of x and y
        double xavg, yavg = 0;

        // iterate and add coordinates
        for (const auto& a : points) {
            Point p = *a;
            xavg += p.getx();
            yavg += p.gety();
        }

        // divide by number of points for average value
        xavg = xavg / numPoints();
        yavg = yavg / numPoints();

        // set center to new center
        this->centerx = xavg;
        this->centery = yavg;

        // return tuple of coordinates
        return old;
    }
};

class Map {
    // how to use map:
    // first load in all the points with addPoint()
    // then make random centers with randCenters()
    // then assign all of them centers with sort()
private:
    vector<Point*> allPoints; // vector of all points on map
    // add a cluster with the given center
    void addCluster(Point *newcenter) {
        allClusters.push_back(new Cluster(newcenter->getx(), newcenter->gety()));
    }

    // puts point in the cluster it belongs
    void clusterize(Point *p) {
        // initialize nearest cluster arbitrary at the first cluster center in the list
        Cluster *nearest = allClusters.at(0);

        // iterate through allCenters and compare distances
        for (const auto& c : allClusters) {
            Cluster *clust = c;
            //cout << "nearest is at " << clust->centerx << "," << clust->centery << "with distance " << p->dist(clust->centerx, clust->centery) << endl;
            if (p->dist(clust->centerx, clust->centery) < p->dist(nearest->centerx, nearest->centery)) {
                nearest = clust;
            }
        }

        // change things only if p's current cluster is not the nearest or the point did not have a cluster
        if (p->clustered == false) {
            nearest->addPoint(p);
        }
        else if (p->clust->centerx != nearest->centerx && p->clust->centery != nearest->centery) {
            // remove p from p's old cluster
            p->clust->removePoint(p);
            // add p to new cluster
            nearest->addPoint(p);
        }

        //cout << "The point " << p->getx() << "," << p->gety() << " has the center " << p->clust->centerx << "," << p->clust->centery << endl;
    }

    // calculate average distance between all points and their center
    double getAvgDist() {
        double avgd = 0;
        for (const auto& a : allClusters) {
            Cluster c = *a;
            avgd += c.setAvgDist();
        }
        avgd = avgd / allClusters.size();
        avgdist = avgd;
        cout << "Average distance is " << avgd << endl;
        return avgd;
    }

    // sort all points into clusters
    void sort() {
        for (const auto& point : allPoints) {
            Point *p = point;
            //cout << "clusterizing " << p->getx() << "," << p->gety() << endl;
            clusterize(p);
        }
        // get clusters' average distance
        getAvgDist();
    }

    // for initiation - assign k random centers that are
    void randCenters(int k) {
        vector<Point*> centers; // store points that have already been selected
        for (int i = 0; i < k; i++) {
            srand(time(0)); // seed based on time for different randomness
            int r = rand() % allPoints.size(); // random int between 0 and size of all (0 included)
            Point *newcenter = allPoints.at(r); // get the selected item
            // check if it is already a center
            if (!centers.empty() && find(centers.begin(), centers.end(), newcenter) != centers.end()) { // if found
                i--; // generate again
            }
            else {
                // add point to centers
                centers.push_back(newcenter);
                addCluster(newcenter); // make a new cluster with the center coordinates
            }
        }
        // deallocate vector
        centers.clear();
        vector<Point*>().swap(centers);
    }

public:

    vector<Cluster*> allClusters; // vector to hold all the centers of clusters
    double avgdist; // average distance from point to its center

    Map() {
        // add in all the points? add a method for this?  Populate currently does this?
    }

    //testing
    void printcenters() {
        for (const auto& c : allClusters) {
            Cluster *clust = c;
            cout << "A center located at coordinate: " << c->centerx << "," << c->centery << endl;
        }
    }

    // add in the beginning: add all points
    void initiate(int k) {
        randCenters(k);
        printcenters();
        sort();
    }

    // add a point
    void addPoint(Point *p) {
        allPoints.push_back(p);
    }

    // reclusters based on new averaged centers of clusters, return whether the center has changed
    bool recluster() {

        bool changed = false; // initialize to unchanged centers

        // loop through all the centers
        for (const auto& c : allClusters) {
            Cluster *clust = c;
            // tuples store x, y coordinates of centers
            tuple<double, double> oldcenter = clust->recenter();
            tuple<double, double> newcenter = make_tuple(clust->centerx, clust->centery);
            cout << "old center: " << get<0>(oldcenter) << "," << get<1>(oldcenter) << "; new center: " << get<0>(newcenter) << "," << get<1>(newcenter) << endl;

            // if the coordinates are different, set changed to true
            if (get<0>(oldcenter) != get<0>(newcenter) || get<1>(oldcenter) != get<1>(newcenter)) {
                changed = true;
                cout << "center has changed." << endl;
            }
        }

        // reseort if changed
        if (changed == true) {
            cout << "centers have changed, re-sort." << endl;
            sort();
        }

        return changed;
    }
};

class KMeans {
    // analysis methods
private:
    int k; // specified number of clusters
    int limit; // the iteration limit of the algorithm
    float xmax = 0, ymax = 0, xmin = 0, ymin = 0;  // used to set the range in gnuplots
    vector<Point*> oldCenters; // stores the previously calculated centers for comparison
    bool changed; // whether or not the centers have changed - ending condition
    string file; // name of input file
    vector<pair<float,float>> data; // set of points for visual plotting
    // add points
    void populate() {
        ifstream infile (file);
        float x, y;
        string line;
        while (getline(infile, line))  {
            istringstream ln(line);
            istream_iterator<string> start(ln), end;
            vector<string> tokens(start,end);
            //cout << "Point located at: (" << tokens.at(tokens.size()-2) << "," << tokens.at(tokens.size()-1) <<  ")\n";
            /* jk not using boost, we now assume that the user (us) is inputting files with the right format
             using boost::lexical_cast;
            using boost::bad_lexical_cast;
            try {
                boost::lexical_cast<float>(tokens.at(tokens.size()-2));
                boost::lexical_cast<float>(tokens.at(tokens.size()-1));
            }
            catch (bad_lexical_cast &) {
                cout << "Please only input files containing points separated by lines and where coordinates are separated by a space" <<  ")\n";
                exit(1);
            }
             */
            x = stof(tokens.at(tokens.size()-2));
            y = stof(tokens.at(tokens.size()-1));
            // finding max and min values
            if (xmax < x) xmax = x; if (xmin > x) xmin = x;
            if (ymax < y) ymax = y; if (ymin > y) ymin = y;

            map->addPoint(new Point(x, y));
            data.emplace_back(x,y);
            cout << "Point located at: (" << x << "," << y <<  ")\n";
        }
        infile.close();
    }

    // initial clustering
    void firstCluster() {
        map->initiate(k);
        Gnuplot gp;
        gp << "set terminal postscript\n";
        gp << "set output \""<< file <<".eps\"\n";
        gp << "set title 'initial' \n";
        gp << "plot ["<<xmin+1<<":"<<xmax+1<<"] ["<<ymin+1<<":"<<ymax+1<<"] '-' tit 'Initial data'\n";
        gp.send1d(data);
        cout << "Xmin: " << xmin << " Xmax: " << xmax << endl;
        cout << "Ymin: " << ymin << " Ymax: " << ymax << endl;
    }

    bool recluster() {
        // calculate new centers based on average coordinates of all points in each cluster, resort points
        cout << "reclustering" << endl;
        return map->recluster();
    }

    void printClusters() {
        ofstream outfile("centered_cleansets/"+to_string(k)+"-centered_"+file); // name original file is inserted
        if (outfile.is_open())  {
            for (const auto& c : map->allClusters) { // The center of each cluster starts with character c, and the coordinates  of the cluster
                Cluster *clust = c;
                outfile << "C:" << c->centerx << " " << c->centery << endl;
                for (const auto&  point: c -> points) {
                    outfile << point->getx() << " " << point->gety() <<  endl;
                }
            }
            outfile.close();
            // for () // The points follow the line with the cluster center, with no prefixes
        }
        else cout << "The file was not opened" << endl;
    }

    // loop the algorithm until centers don't change or the limit is hit
    void doClustering() {
        int i = 0; // counter for limit
        Gnuplot gp;
        gp << "set terminal postscript\n";
        gp << "set output \""<< file <<".eps\"\n";
        while (i < limit && changed) {
            cout << "Iteration " << i+1 << endl;
            changed = recluster();
            cout << "Changed:" << changed << endl;
            i++;
            gp << "plot ["<<xmin+1<<":"<<xmax+1<<"] ["<<ymin+1<<":"<<ymax+1<<"] '-' tit 'Iteration "<< i <<"'\n";
            gp.send1d(data);
        }
        if (!changed) cout << "Clustering limit not reached but k-means converged" << endl;
        else cout << "K-means has not converged but clustering limit has been reached" << endl;
        cout << "Number of iterations: " << i << endl;
        cout << "Xmin: " << xmin << " Xmax: " << xmax << endl;
        cout << "Ymin: " << ymin << " Ymax: " << ymax << endl;
        cout << "Average distance: " << map->avgdist << endl;
    }


public:
    Map *map;
    KMeans(Map *map, int k, int limit, string file) {
        if (k < 1) {
            cout << "K must be a positive integer!" << endl;
            exit(1);
        }
        this->map = map;
        this->k = k;
        this->limit = limit;
        this->file = file;
        changed = true;

        cout << "K = " << k << ", Iteration limit = " << limit << endl;

        populate(); // add all the points
        firstCluster(); // set random clusters and sort
        doClustering(); // do the algorithm
        printClusters(); // outputs the final clustering to an output file, which is named by the original file, prefixed by "k-centered_"
    }

};

// Elbow analysis before selecting k - obtain average distance between point and center over each cluster
class Elbow
{
private:
    // obtain average distance for k, limit
    double getAvgDist(Map *map, int k, int limit, string file) {

        KMeans *km = new KMeans(map, k, limit, file);
        return km->map->avgdist;
    }

public:
    Elbow(Map *map, int limit, string file) {

        // file to write results
        FILE *outf;
        bool f_exists;
        const char * fname = "elbow.txt";

        // file to write results
        outf = fopen(fname, "w+");

        // iterate for many k
        // edit upper bound if necessary
        for (int k = 2; k < 10; k++) {
            fprintf(outf, "Average distance for k=%d: ", k);
            fprintf(outf, "%lf\n", getAvgDist(map, k, limit, file));
        }

        cout << "\nElbow results have been written to elbow.txt." << endl;
        fclose(outf);
    }
};

// use the -e flag for elbow.
int main(int argc, char* argv[]) {

    int k; // the number of clusters
    int limit = pow(4.0,9.0); // the iteration limit
    Map *map = new Map();
    // So k-means is supposed to converge in a finite number of steps
    // (at most k^n) but as a sanity check that our program doesn't loop infinitely we set the iteration limit to k^n
    cout << "Note: File format must contain two floats or integers at the end of each line indicating the coordintates of a point" << endl;
    cout << "Enter the name of the file you'd like to perform k-means on: ";
    string input; getline(cin, input);

    // for elbow
    if (argc > 1 && strcmp(argv[1], "-e") == 0) {
        Elbow *elbow = new Elbow(map, limit, input);
        return 0;
    }

    cout << "Enter the number of desired centroids: ";
    string strk;
    getline(cin, strk);
    KMeans *km = new KMeans(map, stoi(strk), limit, input);
    return 0;
}
