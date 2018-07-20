#ifdef ENABLE_MPI

#ifndef MONITOR_REDUCTION_MPI_HPP_
#define MONITOR_REDUCTION_MPI_HPP_

#include <mpi.h>

#include "Monitor_reduction.hpp"

namespace aff3ct
{
namespace module
{
template <class M> // M is the monitor on which must be applied the reduction
class Monitor_reduction_MPI : public Monitor_reduction_M<M>
{
public:
	Monitor_reduction_MPI(const std::vector<M*> &monitors);
	virtual ~Monitor_reduction_MPI() = default;

	virtual void reset();

protected:
	virtual bool _reduce(bool fully = false, bool last = false);

private:
	MPI_Datatype MPI_monitor_vals;
	MPI_Op       MPI_SUM_monitor_vals;
};
}
}
#include "Monitor_reduction_MPI.hxx"

#endif /* MONITOR_REDUCTION_MPI_HPP_ */

#endif
