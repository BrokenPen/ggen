/* Copyright Swann Perarnau 2009
*
*   contributor(s) :  
*
*   contact : firstname.lastname@imag.fr	
*
* This software is a computer program whose purpose is to help the
* random generation of graph structures and adding various properties
* on those structures.
*
* This software is governed by the CeCILL  license under French law and
* abiding by the rules of distribution of free software.  You can  use, 
* modify and/ or redistribute the software under the terms of the CeCILL
* license as circulated by CEA, CNRS and INRIA at the following URL
* "http://www.cecill.info". 
* 
* As a counterpart to the access to the source code and  rights to copy,
* modify and redistribute granted by the license, users are provided only
* with a limited warranty  and the software's author,  the holder of the
* economic rights,  and the successive licensors  have only  limited
* liability. 
* 
* In this respect, the user's attention is drawn to the risks associated
* with loading,  using,  modifying and/or developing or reproducing the
* software by the user in light of its specific status of free software,
* that may mean  that it is complicated to manipulate,  and  that  also
* therefore means  that it is reserved for developers  and  experienced
* professionals having in-depth computer knowledge. Users are therefore
* encouraged to load and test the software's suitability as regards their
* requirements in conditions enabling the security of their systems and/or 
* data to be ensured and,  more generally, to use and operate it in the 
* same conditions as regards security. 
* 
* The fact that you are presently reading this means that you have had
* knowledge of the CeCILL license and that you accept its terms.
*/
/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#include <iostream>

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>

#include <boost/program_options.hpp>

#include <iostream>


/* GSL statistic functions
 * GSL is prefered over boost accumulators as boost seems to
 * compute statistics by approximation using a cache of values
 */
#include "gsl/gsl_statistics.h"


#include "ggen.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

dynamic_properties properties(&create_property_map);
Graph *g;

/* Main program
 */
int main(int argc, char** argv)
{
	istream *infile = NULL;
	bool edge_property;
	string name;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce this help message")
		
		/* I/O options */
		("input,i", po::value<string>(), "Set the input file")

		/* Property options */
		("name,n",po::value<string>(),"Set the property name to analyse")
		("edge,e",po::value<bool>()->zero_tokens(),"Analyse an edge property instead of a vertex one")
	;

	po::options_description all;
	all.add(desc);
	
	
	// Parse command line options
	////////////////////////////////
	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,all),vm);
	po::notify(vm);
	
	if (vm.count("help"))
	{
		std::cout << all << "\n";
		        return 1;
	}

	if (vm.count("input")) 
	{
		filebuf *fb = new filebuf();
		fb->open(vm["input"].as<std::string>().c_str(),ios::in);
		infile = new istream(fb);
	}
	else
		infile = &cin;

	if(vm.count("name"))
	{
		name = vm["name"].as<string>();
	}
	else
		name = "node_id";

	if(vm.count("edge"))
	{
		edge_property = true;
	}
	else
		edge_property = false;

	// Graph generation
	////////////////////////////////
	
	g = new Graph();

	// Read graph
	////////////////////////////////////	
	read_graphviz(*infile, *g,properties);
	
	// Statistics
	// What todo : 	push_back values needed
	// 		convert the vector to call gsl
	// 		print the values
	double mean;
	std::vector<double> values;
	if(edge_property)
	{
		// iterate over edges and push_back values
		std::pair<Edge_iter, Edge_iter> ep;
		for (ep = boost::edges(*g); ep.first != ep.second; ++ep.first)
			values.push_back(boost::lexical_cast<double>(get(name,properties,*ep.first)));
	}
	else
	{
		// iterate over vertices and push_back values
		std::pair<Vertex_iter, Vertex_iter> vp;
		for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
			values.push_back(boost::lexical_cast<double>(get(name,properties,*vp.first)));
	}

	double *data = new double [values.size()];
	copy( values.begin(), values.end(), data);
	
       	mean = gsl_stats_mean(data, 1, values.size());
	
	std::cout << "Statistics for property " << name << " \n";
	std::cout << "Mean : " << mean << " \n";
	std::cout << "Standard Deviation : " << gsl_stats_sd_m(data,1,values.size(),mean) << " \n";	

	delete [] data;
	delete g;
	return 0;
}