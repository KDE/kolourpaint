Entry Point
~~~~~~~~~~~
kolourpaint.cpp:main() initializes KolourPaint and creates a whole bunch of
kpMainWindow's.




Package Overview
~~~~~~~~~~~~~~~~


Class Overview
~~~~~~~~~~~~~~

kpCommnd can only exist while there is a kpDocument and kpView and
kpViewManager and kpTool.
However, a kpSelection might not exist.  This is a good idea as to what
accessors may return null pointers.

kpCommand and kpDocument are not GUI classes.  They should not access the
kpMainWindow, kp*ToolBar or kpTool.

kpTool creates kpCommand.

TODO: Highlight main classes


kpDefs.h


kpThumbnail


kpViewScrollableContainer.
