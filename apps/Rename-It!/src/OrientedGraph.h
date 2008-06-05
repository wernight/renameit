/** Oriented graph concrete class.
 * Copyright (c) 2005 Werner BEROUX
 */

#pragma once

#include <vector>
#include <map>

namespace Beroux{ namespace Math{
	class OrientedGraph
	{
	public:
		class Node;

		class Iterator
		{
		public:
		// Construction
			Iterator(void) {}
			Iterator(std::map<unsigned, Node>::iterator	iter) : m_iter(iter) {}
			~Iterator(void) {}

		// Operations
			inline void operator=(const Iterator& that) {
				m_iter = that.m_iter;
			}

			inline void operator++(void) {
				++m_iter;
			}

			inline bool operator!=(const Iterator& other) {
				return other.m_iter != m_iter;
			}

			inline unsigned operator*(void) {
				return m_iter->first;
			}

		// Implementation
		private:
			std::map<unsigned, Node>::iterator	m_iter;
		};

		class Node
		{
		public:
		// Construction
			Node(unsigned nNode, OrientedGraph& g) : 
				m_nNode(nNode),
				m_graph(g)
			{}

			~Node(void) {};

		// Attributes
			inline bool HasSuccessor(void) const
			{
				return !m_vSuccessors.empty();
			}

			inline bool HasAntecedent(void) const
			{
				return !m_vAntecedents.empty();
			}

			inline unsigned GetSuccessorsCount(void)
			{
				return (unsigned) m_vSuccessors.size();
			}

			inline unsigned GetAntecedentsCount(void)
			{
				return (unsigned) m_vAntecedents.size();
			}

			inline unsigned GetSuccessor(unsigned n) const
			{
				return m_vSuccessors[n];
			}

			inline unsigned GetAntecedent(unsigned n) const
			{
				return m_vAntecedents[n];
			}

		// Operations
			bool AddSuccessor(unsigned nNode)
			{
				try {
					m_graph[nNode].m_vAntecedents.push_back(m_nNode);
					m_vSuccessors.push_back(nNode);
					return true;
				}
				catch (...) {
					return false;
				}
			}

			bool RemoveSuccessor(unsigned n)
			{
				if (n >= m_vSuccessors.size())
					return false;

				// Remove successor's antecedent to this node
				std::vector<unsigned>& succAnt = m_graph[m_vSuccessors[n]].m_vAntecedents;
				std::vector<unsigned>::iterator iter;
				for (iter=succAnt.begin(); *iter != m_nNode; ++iter)
					ASSERT(iter != succAnt.end());
				succAnt.erase( iter );

				// Remove successor
				for (iter=m_vSuccessors.begin(); n > 0; ++iter, --n)
					ASSERT(iter != m_vSuccessors.end());
				m_vSuccessors.erase( iter );

				return true;
			}

			virtual void RemoveAllSuccessors(void)
			{
				while (HasSuccessor())
					RemoveSuccessor(0);
			}

		// Implementation
		protected:
			unsigned				m_nNode;	// This node's number
			OrientedGraph&			m_graph;	// Link to the graph
			std::vector<unsigned>	m_vSuccessors,
									m_vAntecedents;
		};

	// Construction
		OrientedGraph(void) {};
		~OrientedGraph(void) {};

	// Attributes
		Node& GetNode(unsigned nNode)
		{
			return m_graph.find(nNode)->second;
		}

		const Node& GetNode(unsigned nNode) const
		{
			return m_graph.find(nNode)->second;
		}

		inline Node& operator[](unsigned nNode)
		{
			return GetNode(nNode);
		}

		inline const Node& operator[](unsigned nNode) const
		{
			return GetNode(nNode);
		}

		const Iterator& Begin(void)
		{
			static const Iterator iter( m_graph.begin() );
			return iter;
		}

		const Iterator& End(void)
		{
			static const Iterator iter( m_graph.end() );
			return iter;
		}

	// Operations
		/** Add a new node (that has no successor and no antecedent).
		* \return true is it's successfull, false if the node exists already.
		*/
		bool AddNode(unsigned nNode)
		{
			std::map<unsigned, Node>::iterator iterAt = m_graph.find(nNode);
			if (iterAt != m_graph.end())
				return false;
			std::pair<unsigned, Node>	p(nNode, Node(nNode, *this));
            m_graph.insert(iterAt, p);
			return true;
		}

		/** Remove a node.
		* \return true is it's successfull, false if the node doesn't exist.
		*/
		bool RemoveNode(unsigned nNode)
		{
			std::map<unsigned, Node>::iterator iterAt = m_graph.find(nNode);
			if (iterAt != m_graph.end())
				return false;
			iterAt->second.RemoveAllSuccessors();
			m_graph.erase(iterAt);
			return true;
		}

	// Implementation
	private:
		std::map<unsigned, Node>	m_graph;
	};
}}
