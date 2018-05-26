# pico

Pico is a toy proof-of-concept tool for managing text buffers effeciently. Text is stored in a splay tree, based on a piece table scheme (you can find a full explanation [here](http://www.averylaird.com/programming/piece-table/2018/05/10/insertions-piece-table/)).

Pico is in the "barely works" phase, and only supports insertions. You can run the small curses interface to play around with it (the linking should be clear from `CMakeLists.txt`).

# Want to Contribute?

Any and all help is certainly welcome. If you'd like to contribute, the best way is to fork this repo and submit a pull request. Here are some general goals for the future:

* deletions
* undo
* multiple cursors
* multiple users
