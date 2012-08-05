Treeview drag-drop example
--------------------------

The example shows a typical full-window application with a tree (a
`WTreeView`) that represents different 'folders', and a table (a
`WTableView`) which represents 'documents' that belong to this
office. The size of these documents is shown in a pie chart
(a `WPieChart`).

Two data models (`WStandardItemModel`) back these views: one for the
folders, and one for the files.

Drag & Drop is configured so that files can be selected and dragged
into a different folder. Changes to the model are automatically
reflected in the relevant views.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

- Standard Item views and models
- Drag & Drop with standard item models
- Using a `WPopupMenu`
- Using a dialog to edit data