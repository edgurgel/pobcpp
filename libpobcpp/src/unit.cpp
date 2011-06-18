#include "unit.h"

#include "unit_type_serialization.h"
#include "basic_communicator.h"
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/group.hpp>
#include <boost/serialization/map.hpp> //std::pair serialize
#include <boost/mpi/environment.hpp>

namespace Pobcpp {

Unit::Unit (void) { 
	comm = new Basic_Communicator;
}

Unit::~Unit (void) { }


//FIXME Include possible Communicator argument on the grammar ranksof() function.

unsigned int* Unit::ranksof_impl(Basic_Communicator comm, Unit_Type& asked_type, Unit_Type& unit_type) {
// Constructing the array.
	boost::mpi::communicator world(comm.get_mpi_comm(), boost::mpi::comm_attach);
	unsigned int rank = world.rank();

	std::vector<Unit_Type_Pack> types;
	all_gather(world, Unit_Type_Pack(unit_type, rank), types); // send the pair<unit_type,world.rank()>
	unsigned int size = 0;
	std::vector<unsigned int> ranks;
	for(int i = 0; i < world.size(); ++i) {
		Unit_Type_Pack pack_i = types[i];
		unsigned int ranki = pack_i.rank;
		Unit_Type typei = pack_i.unit_type;
		if(asked_type == typei) {
			ranks.push_back(ranki);
			size++;
		}
	}
	unsigned int* ranks_result = new unsigned int[size];
	memcpy(ranks_result, &(ranks[0]), size * sizeof(unsigned int));
	return ranks_result;
}

}
