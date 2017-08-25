/*!
 * \file
 * \brief A Decoder is an algorithm dedicated to find the initial sequence of information bits (before the noise).
 *
 * \section LICENSE
 * This file is under MIT license (https://opensource.org/licenses/MIT).
 */
#ifndef DECODER_HIHO_HPP_
#define DECODER_HIHO_HPP_

#include <chrono>
#include <string>
#include <vector>
#include <sstream>

#include "Tools/Exception/exception.hpp"

#include "Decoder.hpp"

namespace aff3ct
{
namespace module
{
/*!
 * \class Decoder_SIHO_i
 *
 * \brief A Decoder is an algorithm dedicated to find the initial sequence of information bits (before the noise).
 *
 * \tparam B: type of the bits in the Decoder.
 * \tparam R: type of the reals (floating-point or fixed-point representation) in the Decoder.
 *
 * The Decoder takes a soft input (real numbers) and return a hard output (bits).
 * Please use Decoder for inheritance (instead of Decoder_HIHO_i).
 */
template <typename B = int>
class Decoder_HIHO_i : virtual public Decoder
{
private:
	std::vector<B> Y_N;
	std::vector<B> V_KN;

public:
	/*!
	 * \brief Constructor.
	 *
	 * \param K:                      number of information bits in the frame.
	 * \param N:                      size of one frame.
	 * \param n_frames:               number of frames to process in the Decoder.
	 * \param simd_inter_frame_level: number of frames absorbed by the SIMD instructions.
	 * \param name:                   Decoder's name.
	 */
	Decoder_HIHO_i(const int K, const int N, const int n_frames = 1, const int simd_inter_frame_level = 1,
	               std::string name = "Decoder_HIHO_i")
	: Decoder(K, N, n_frames, simd_inter_frame_level, name),
	  Y_N    (this->n_inter_frame_rest ? this->simd_inter_frame_level * this->N : 0),
	  V_KN   (this->n_inter_frame_rest ? this->simd_inter_frame_level * this->N : 0)
	{
		auto &p1 = this->create_process("decode_hiho");
		this->template create_socket_in <B>(p1, "Y_N", this->N * this->n_frames);
		this->template create_socket_out<B>(p1, "V_K", this->K * this->n_frames);
		this->create_codelet(p1, [&]() -> int
		{
			this->decode_hiho(static_cast<B*>(p1["Y_N"].get_dataptr()),
			                  static_cast<B*>(p1["V_K"].get_dataptr()));

			return 0;
		});
		this->register_duration(p1, "load");
		this->register_duration(p1, "decode");
		this->register_duration(p1, "store");

		auto &p2 = this->create_process("decode_hiho_coded");
		this->template create_socket_in <B>(p2, "Y_N", this->N * this->n_frames);
		this->template create_socket_out<B>(p2, "V_N", this->N * this->n_frames);
		this->create_codelet(p2, [&]() -> int
		{
			this->decode_hiho_coded(static_cast<B*>(p2["Y_N"].get_dataptr()),
			                        static_cast<B*>(p2["V_N"].get_dataptr()));

			return 0;
		});
		this->register_duration(p2, "load");
		this->register_duration(p2, "decode");
		this->register_duration(p2, "store");
	}

	/*!
	 * \brief Destructor.
	 */
	virtual ~Decoder_HIHO_i()
	{
	}

	/*!
	 * \brief Decodes the noisy frame.
	 *
	 * \param Y_N: a noisy frame.
	 * \param V_K: a decoded codeword (only the information bits).
	 */
	template <class A = std::allocator<B>>
	void decode_hiho(const std::vector<B,A>& Y_N, std::vector<B,A>& V_K)
	{
		if (this->N * this->n_frames != (int)Y_N.size())
		{
			std::stringstream message;
			message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = " << Y_N.size()
			        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
			throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
		}

		if (this->K * this->n_frames != (int)V_K.size())
		{
			std::stringstream message;
			message << "'V_K.size()' has to be equal to 'K' * 'n_frames' ('V_K.size()' = " << V_K.size()
			        << ", 'K' = " << this->K << ", 'n_frames' = " << this->n_frames << ").";
			throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
		}

		this->decode_hiho(Y_N.data(), V_K.data());
	}

	virtual void decode_hiho(const B *Y_N, B *V_K)
	{
		auto w = 0;
		for (w = 0; w < this->n_dec_waves -1; w++)
			this->_decode_hiho(Y_N + w * this->N * this->simd_inter_frame_level,
			                   V_K + w * this->K * this->simd_inter_frame_level,
			                   w * this->simd_inter_frame_level);

		if (this->n_inter_frame_rest == 0)
			this->_decode_hiho(Y_N + w * this->N * this->simd_inter_frame_level,
			                   V_K + w * this->K * this->simd_inter_frame_level,
			                   w * this->simd_inter_frame_level);
		else
		{
			const auto waves_off1 = w * this->simd_inter_frame_level * this->N;
			std::copy(Y_N + waves_off1,
			          Y_N + waves_off1 + this->n_inter_frame_rest * this->N,
			          this->Y_N.begin());

			this->_decode_hiho(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);

			const auto waves_off2 = w * this->simd_inter_frame_level * this->K;
			std::copy(this->V_KN.begin(),
			          this->V_KN.begin() + this->n_inter_frame_rest * this->K,
			          V_K + waves_off2);
		}
	}

	template <class A = std::allocator<B>>
	void decode_hiho_coded(const std::vector<B,A>& Y_N, std::vector<B,A>& V_N)
	{
		if (this->N * this->n_frames != (int)Y_N.size())
		{
			std::stringstream message;
			message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = " << Y_N.size()
			        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
			throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
		}

		if (this->N * this->n_frames != (int)V_N.size())
		{
			std::stringstream message;
			message << "'V_N.size()' has to be equal to 'N' * 'n_frames' ('V_N.size()' = " << V_N.size()
			        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
			throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
		}

		this->decode_hiho_coded(Y_N.data(), V_N.data());
	}

	virtual void decode_hiho_coded(const B *Y_N, B *V_N)
	{
		auto w = 0;
		for (w = 0; w < this->n_dec_waves -1; w++)
			this->_decode_hiho_coded(Y_N + w * this->N * this->simd_inter_frame_level,
			                         V_N + w * this->N * this->simd_inter_frame_level,
			                         w * this->simd_inter_frame_level);

		if (this->n_inter_frame_rest == 0)
			this->_decode_hiho_coded(Y_N + w * this->N * this->simd_inter_frame_level,
			                         V_N + w * this->N * this->simd_inter_frame_level,
			                         w * this->simd_inter_frame_level);
		else
		{
			const auto waves_off1 = w * this->simd_inter_frame_level * this->N;
			std::copy(Y_N + waves_off1,
			          Y_N + waves_off1 + this->n_inter_frame_rest * this->N,
			          this->Y_N.begin());

			this->_decode_hiho_coded(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);

			const auto waves_off2 = w * this->simd_inter_frame_level * this->N;
			std::copy(this->V_KN.begin(),
			          this->V_KN.begin() + this->n_inter_frame_rest * this->N,
			          V_N + waves_off2);
		}
	}

protected:
	virtual void _decode_hiho(const B *Y_N, B *V_K, const int frame_id)
	{
		throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
	}

	virtual void _decode_hiho_coded(const B *Y_N, B *V_N, const int frame_id)
	{
		throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
	}
};
}
}

#include "SC_Decoder_HIHO.hpp"

#endif /* DECODER_HIHO_HPP_ */