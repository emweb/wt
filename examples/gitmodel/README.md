Git example
-----------

This example shows a git repository browser. It shows the contents of
the master version of a git repository, using low-level git commands
to query the git repository.

It illustrates how to build a read-only custom abstract item model.

How to run
----------

See the README in the parent directory.

You can set the GITVIEW_REPOSITORY_PATH environment variable to indicate
a default repository location (which should be a bare repository or the
'.git' folder within a normal repository).

What it illustrates
-------------------

- how to specialize WAbstractItemModel to implement tree-like models
- the use of layout managers for a window-filling layout
- the use of WTreeView with a custom item model
