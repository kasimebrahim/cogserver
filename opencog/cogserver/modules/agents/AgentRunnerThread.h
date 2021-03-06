/*
 * opencog/cogserver/modules/agents/AgentRunnerThread.h
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef OPENCOG_SERVER_AGENTRUNNERTHREAD_H_
#define OPENCOG_SERVER_AGENTRUNNERTHREAD_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <opencog/cogserver/modules/agents/AgentRunnerBase.h>

namespace opencog
{

/**
 * This class uses a worker thread to run one or more agents continuously. The
 * Agent::run() function of all agents are called in a round robin manner.
 *
 * You can call start() to enable running agents, and stop() to disable it.
 *
 * The worker thread is created when the first agent is added, and terminates
 * when there are no agents to run.
 */
class AgentRunnerThread: public AgentRunnerBase
{
    public:
        AgentRunnerThread(const std::string &name = "agent_thread");
        /*
         * AgentRunnerThread is not movable anyway, because mutex &
         * condition_variable are not. We just make it explicit.
         */
        AgentRunnerThread(AgentRunnerThread &&tmp) = delete;
        ~AgentRunnerThread();

        /** Start running agents */
        void start();

        /** Stop running agents */
        void stop();

        /** Adds agent 'a' to the list of scheduled agents. */
        void add_agent(AgentPtr a);

        /** Removes agent 'a' from the list of scheduled agents. */
        void remove_agent(AgentPtr a);

        /** Removes all agents from class 'id' */
        void remove_all_agents(const std::string &id);

        void remove_all_agents();

        AgentSeq get_agents() const;

        /**
         * @return if any agents are controlled by this runner object
         */
        bool has_agents() const;

        void set_activity_table(SystemActivityTable* sat) { this->sat = sat; };

    private:
        /** If running agents is enabled */
        std::atomic_bool running;

        /** Mutex for running_cond */
        std::mutex running_cond_mutex;

        /** Condition variable on running, used to wake up worker thread
         * when running becomes true */
        std::condition_variable running_cond;

        /** The worker thread */
        std::thread run_thread;

        /** If the list of scheduled agents should be modified while run_thread
         * is active */
        std::atomic_bool agents_modified;

        /** Protects AgenRunnerBase::agents */
        mutable std::mutex agents_mutex;

        /** Agents to be added to the list of scheduled agents */
        std::vector<AgentPtr> agents_add_q;

        /** Agents to be removed from the list of scheduled agents */
        std::vector<AgentPtr> agents_remove_q;

        /** Agent IDs to be removed from the list of scheduled agents */
        std::vector<std::string> ids_remove_q;

        /** If the list of scheduled agents should be cleared */
        bool clear_all;

        /** The function which runs in the worker thread, and runs all agents
         * while running agents is enabled. This function terminates when
         * there are no agents to run.
         */
        void process_agents_thread();

        /** If possible, wakes up the worker thread (when running is disabled)
         * and waits for its termination and joins the thread.
         */
        void join_run_thread();
};

} /* namespace opencog */

#endif /* OPENCOG_SERVER_AGENTRUNNERTHREAD_H_ */
