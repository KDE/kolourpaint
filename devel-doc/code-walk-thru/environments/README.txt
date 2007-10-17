
environments/ Package


This is the software engineering "Facade" pattern.  It tries to centralize
interactions between major packages (e.g. the Tool and Main Window
packages) to avoid many-to-many relationships between key classes.

For instance, in KolourPaint/KDE3 where there were no environments,
a sample of the interactions between classes looked like this:

    kpToolBrush   kpToolEllipse
           |        |
           V        V
          kpMainWindow <-- kpToolEllipseCommand
           ^
           |
    kpToolBrushCommand

[the direction of the arrows show that kpToolBrush and friends access
 kpMainWindow; although not shown in the diagram, note that kpMainWindow
 is assumed to access all these classes]

As almost every class accessed the Main Window, it was almost impossible
to change the interface of the Main Window without adapting tens of files
or at least, waiting for them to recompile.  Similarly, other classes
suffered from this problem.

In KolourPaint/KDE4, many classes access other packages through
"Environment" classes.  Our example above transforms into:

    kpToolBrush   kpToolEllipse
           |        |
           V        V
       [ kpToolEnvironment ]
               |
               V
           kpMainWindow
               ^
               |
       [ kpCommandEnvironment ] <-- kpToolEllipseCommand
           ^
           |
    kpToolBrushCommand

[kpMainwindow still accesses all classes but at least, very few classes
 now access kpMainWindow]

Virtually every package now accesses the Main Window (and other packages)
through environments.  This reduces coupling, increasing code agility.

The hierarchy of this package is a subset of the top-level hierarchy.
All environments are derived from kpEnvironmentBase and are stored by
kpMainWindow.  The naming is as expected e.g. kpToolEnvironment is used
by classes dervied from kpTool to access other classes.
