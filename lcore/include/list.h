
#pragma once

#include <fastmemmanager.h>

template< class T > 
struct ListNode
{
   T value;
   ListNode* next;
   ListNode* prev;
};

template< class T >
class List
{
   public:
                List( bool fast = true ): fastmem(fast), mem(fast?1024:0,2), first(0), last(0), count(0) {}

   ListNode<T>* front() { return first; }
   ListNode<T>* back() { return last; }

           bool empty() const { return first==0; }
           int  size()  const { return count; }

           void pop_front() { erase( &first ); }
           void push_front( const T& elem ) { push_before( first, elem ); }
           void push_back( const T& elem ) { push_after( last, elem ); }

           void push_before( ListNode<T>* node, const T& elem ) { push( node, elem, false ); }
           void push_after( ListNode<T>* node, const T& elem ) { push( node, elem, true ); }

           void clear() { while( !empty() ) pop_front(); }

           void erase( ListNode<T>** node )
           {
             if( !*node ) return;
 
             ListNode<T> *next = (*node)->next,
                         *prev = (*node)->prev;
 
             fastmem ? mem.Free( *node ) : free( *node );
             *node = 0;
  
             if( prev )
               prev->next = next;
             else
               first = next;
 
             if( next )
               next->prev = prev;
             else
               last = prev;

             --count;
           }

   private:
      FastMemManager< ListNode<T> > mem;
      ListNode<T>*  first;
      ListNode<T>*  last;
      const bool    fastmem; // use fast memory manager
      int           count;

          void push( ListNode<T>* node, const T& elem, bool after )
          {
             ListNode<T> *p, *prev = 0, *next = 0;

             p = fastmem ? mem.Allocate() : (ListNode<T>*)malloc( sizeof(ListNode<T>) );
             if( !p )   return;

             if( node )
             {
               prev = node->prev;
               next = node->next;
             }

             p->value = elem;
             if( after )
             {
               p->next = next;
               p->prev = node;
             }
             else
             {
               p->next = node;
               p->prev = prev;
             }

             if( p->next )
               p->next->prev = p;

             if( p->prev )
               p->prev->next = p;

             if( !p->next ) last = p;
             if( !p->prev ) first = p;

             ++count;
          }
};

template< class T >
class Queue: public List<T>
{
   public:
           Queue( bool fast = true ): List<T>(fast) {}

      void push( const T& elem ) { push_back( elem ); }
      void pop() { List<T>::pop_front(); }
};

