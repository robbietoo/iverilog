#ifndef __event_H
#define __event_H
/*
 * Copyright (c) 2004-2008 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "vvp_net.h"
# include  "pointers.h"

class evctl {

    public:
      explicit evctl(unsigned long ecount);
      bool dec_and_run();
      virtual void run_run() = 0;
      virtual ~evctl() {}
      evctl*next;

    protected:
      unsigned long ecount_;
};

class evctl_real : public evctl {

    public:
      explicit evctl_real(struct __vpiHandle*handle, double value,
                          unsigned long ecount);
      virtual ~evctl_real() {}
      void run_run();

    private:
      __vpiHandle*handle_;
      double value_;
};

class evctl_vector : public evctl {

    public:
      explicit evctl_vector(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                            unsigned off, unsigned wid, unsigned long ecount);
      virtual ~evctl_vector() {}
      void run_run();

    private:
      vvp_net_ptr_t ptr_;
      vvp_vector4_t value_;
      unsigned off_;
      unsigned wid_;
};

extern void schedule_evctl(struct __vpiHandle*handle, double value,
                           vvp_net_t*event, unsigned long ecount);

extern void schedule_evctl(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                           unsigned offset, unsigned wid,
                           vvp_net_t*event, unsigned long ecount);

/*
 *  Event / edge detection functors
 */

/*
 * A "waitable" functor is one that the %wait instruction can wait
 * on. This includes the infrastructure needed to hold threads.
 */
struct waitable_hooks_s {

    public:
      waitable_hooks_s() : threads(0), event_ctls(0) { last = &event_ctls; }
      vthread_t threads;
      evctl*event_ctls;
      evctl**last;

    protected:
      void run_waiting_threads_();
};

/*
 * The vvp_fun_edge functor detects events that are edges of various
 * types. This should be hooked to a vvp_net_t that is connected to
 * the output of a signal that we wish to watch for edges.
 */
class vvp_fun_edge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      typedef unsigned short edge_t;
      explicit vvp_fun_edge(edge_t e, bool debug_flag);

      virtual ~vvp_fun_edge();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      vvp_bit4_t bits_[4];
      edge_t edge_;
      bool debug_;
};

extern const vvp_fun_edge::edge_t vvp_edge_posedge;
extern const vvp_fun_edge::edge_t vvp_edge_negedge;
extern const vvp_fun_edge::edge_t vvp_edge_none;

/*
 * The vvp_fun_anyedge functor checks to see if any value in an input
 * vector changes. Unlike the vvp_fun_edge, which watches for the LSB
 * of its inputs to change in a particular direction, the anyedge
 * functor looks at the entire input vector for any change.
 *
 * The anyedge is also different in that it can receive real
 * values. in this case, any detectable change in the real value leads
 * to an even trigger.
 */
class vvp_fun_anyedge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_anyedge(bool debug_flag);
      virtual ~vvp_fun_anyedge();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
      void recv_real(vvp_net_ptr_t port, double bit);

    private:
      bool debug_;
      vvp_vector4_t bits_[4];
	// In case I'm a real-valued event.
      double bitsr_[4];
};

/*
 * This functor triggers anytime any input is set, no matter what the
 * value. This is similar to a named event, but it has no handle.
 */
class vvp_fun_event_or : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_event_or();
      ~vvp_fun_event_or();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
};

/*
 * A named event is simpler then a vvp_fun_edge in that it triggers on
 * any input at all to port-0. The idea here is that behavioral code
 * can use a %set/v instruction to trigger the event.
 */
class vvp_named_event : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_named_event(struct __vpiHandle*eh);
      ~vvp_named_event();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      struct __vpiHandle*handle_;
};

#endif // __event_H
