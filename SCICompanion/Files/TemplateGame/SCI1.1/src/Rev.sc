(version 2)
(include "sci.sh")
(include "game.sh")
(use "Cycle")
(script REVERSECYCLE_SCRIPT)

(class Rev of Cycle
    (properties
        client 0
        caller 0
        cycleDir -1
        cycleCnt 0
        completed 0
    )

    (method (doit)
        (var revNextCel)
        = revNextCel (self:nextCel())
        (if (< revNextCel 0)
            (self:cycleDone())
        )(else
            (send client:cel(revNextCel))
        )
    )


    (method (cycleDone)
        (send client:cel((send client:lastCel())))
    )

)
