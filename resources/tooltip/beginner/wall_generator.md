#### Wall Generator

**What it does:** Chooses the algorithm for generating wall paths -- Classic uses a fixed line width, while Arachne varies the width to fill spaces more accurately.

**Why it matters:** Arachne produces cleaner results on thin features and odd-width sections by adjusting extrusion width to perfectly fill the available space, reducing gaps and overlaps.

**Typical values:** Classic (simple and predictable) or Arachne (better quality on detailed models).

**Tip:** Use Arachne for models with thin walls or fine details -- it handles tricky geometry much better than Classic and is the recommended default.
