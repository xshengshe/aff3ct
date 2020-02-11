/*!
 * \file
 * \brief Class module::Task.
 */
#ifndef TASK_HPP_
#define TASK_HPP_

#include <functional>
#include <cstddef>
#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <mipp.h>

#include "Tools/Interface/Interface_clone.hpp"
#include "Tools/Interface/Interface_reset.hpp"

namespace aff3ct
{
namespace module
{
class Module;
class Socket;

enum class socket_t : uint8_t { SIN, SIN_SOUT, SOUT };

class Task : public tools::Interface_clone, public tools::Interface_reset
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	friend Socket;
	friend Module;
#endif

protected:
	Module *module;
	const std::string name;
	bool autoalloc;
	bool stats;
	bool fast;
	bool debug;
	bool debug_hex;
	int32_t debug_limit;
	uint8_t debug_precision;
	int32_t debug_frame_max;
	std::function<int(Module &m, Task& t)> codelet;

	std::vector<int> status;
	std::vector<mipp::vector<uint8_t>> out_buffers;

	// stats
	uint32_t                 n_calls;
	std::chrono::nanoseconds duration_total;
	std::chrono::nanoseconds duration_min;
	std::chrono::nanoseconds duration_max;

	std::vector<std::string             > timers_name;
	std::vector<uint32_t                > timers_n_calls;
	std::vector<std::chrono::nanoseconds> timers_total;
	std::vector<std::chrono::nanoseconds> timers_min;
	std::vector<std::chrono::nanoseconds> timers_max;

	Socket* last_input_socket;
	std::vector<socket_t> socket_type;

public:
	std::vector<std::shared_ptr<Socket>> sockets;

	Task(      Module &module,
	     const std::string &name,
	     const bool autoalloc = true,
	     const bool stats     = false,
	     const bool fast      = false,
	     const bool debug     = false);

	virtual ~Task() = default;

	void reset();

	void set_autoalloc      (const bool     autoalloc);
	void set_stats          (const bool     stats    );
	void set_fast           (const bool     fast     );
	void set_debug          (const bool     debug    );
	void set_debug_hex      (const bool     debug_hex);
	void set_debug_limit    (const uint32_t limit    );
	void set_debug_precision(const uint8_t  prec     );
	void set_debug_frame_max(const uint32_t limit    );

	inline bool is_autoalloc        (                  ) const;
	inline bool is_stats            (                  ) const;
	inline bool is_fast             (                  ) const;
	inline bool is_debug            (                  ) const;
	inline bool is_debug_hex        (                  ) const;
	inline bool is_last_input_socket(const Socket &s_in) const;
	       bool can_exec            (                  ) const;

	inline Module&       get_module     (               ) const;
	inline std::string   get_name       (               ) const;
	inline uint32_t      get_n_calls    (               ) const;
	       socket_t      get_socket_type(const Socket &s) const;

	// get stats
	std::chrono::nanoseconds                     get_duration_total() const;
	std::chrono::nanoseconds                     get_duration_avg  () const;
	std::chrono::nanoseconds                     get_duration_min  () const;
	std::chrono::nanoseconds                     get_duration_max  () const;
	const std::vector<std::string>             & get_timers_name   () const;
	const std::vector<uint32_t>                & get_timers_n_calls() const;
	const std::vector<std::chrono::nanoseconds>& get_timers_total  () const;
	const std::vector<std::chrono::nanoseconds>& get_timers_min    () const;
	const std::vector<std::chrono::nanoseconds>& get_timers_max    () const;

	int exec();

	inline Socket& operator[](const size_t id);

	inline void update_timer(const size_t id, const std::chrono::nanoseconds &duration);

	Task* clone() const;

protected:
	void register_timer(const std::string &key);

	template <typename T>
	size_t create_socket_in(const std::string &name, const size_t n_elmts);

	template <typename T>
	size_t create_socket_in_out(const std::string &name, const size_t n_elmts);

	template <typename T>
	size_t create_socket_out(const std::string &name, const size_t n_elmts, const bool hack_status = false);

	void create_codelet(std::function<int(Module &m, Task& t)> &codelet);

private:
	template <typename T>
	inline Socket& create_socket(const std::string &name, const size_t n_elmts, const bool hack_status = false);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Task.hxx"
#endif

#endif /* TASK_HPP_ */
