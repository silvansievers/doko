Description
===========

doko is a C++ doppelkopf program with an integrated UCT player..

This code base makes two contributions:

* First, it provides an implementation of the game of
  `Doppelkopf <https://en.wikipedia.org/wiki/Doppelkopf>`_. Only
  official (tournament) rules are supported.

* Second, it provides an implementation of a computer player based on
  the UCT algorithm. Humans can also play via console input.

The implementation has been described and evaluated in detail in
`Implementation of the UCT Algorithm for Doppelkopf
<http://ai.cs.unibas.ch/papers/sievers-master-12.pdf>`_ (Silvan Sievers,
University of Freiburg, 2012) and also published in `A Doppelkopf
Player Based on UCT
<http://ai.cs.unibas.ch/papers/sievers-helmert-ki2015.pdf>`_ (Silvan
Sievers and Malte Helmert, KI 2015).

Notes on the code:

* I developed this code during my Master's thesis and it is by no
  means in a "finished state". There is plenty room for improving this
  code, e.g. by making it more modular (in general by using a better
  design), using shorter methods etc. Several places contain copy &
  pasted parts, emerged due to time pressure :-)

* That being said, this doesn't imply that one should by default assume
  that the code contains tons of bugs; experiments and lots of human
  playing against the UCT player revealed no more bugs in the end (of
  course there will always be, but hopefully not in a high amount),
  and it is very reasonable to assume that the game is correctly
  implemented.

* I publish the code nevertheless, because since 2012, several people
  wished to have access to the code, and I always thought I would first
  go to clean it up. Being realistic, this will not happen in the near
  future, and so I hope the code can still be useful as it is.

* Finally, if you decide to build upon the code base and find anything
  suspicious to be a bug, please let me know! You may just email me or
  create issues on bitbucket.

Compilation
===========

Simply typing ``make`` should work on modern Unix systems.

Usage
=====

Type ``./doko --help`` to get an overview of available commands.
Running ``./doko`` will start a game without a human player, letting a
UCT player play against three random players. A standard tournament set
of 24 games against the presumably best configuration of a UCT player
can be played using the following command: ``./doko -n 24 -r
--announcing-version 1 --compulsory-solo --verbose -p human uct uct uct
--p1-options 0 500 1 1 16000 1000 10 1 0 0 1 --p2-options 0 500 1 1
16000 1000 10 1 0 0 1 --p3-options 0 500 1 1 16000 1000 10 1 0 0 1``


Contact
=======

For questions, please contact Silvan Sievers: silvan.sievers@unibas.ch
