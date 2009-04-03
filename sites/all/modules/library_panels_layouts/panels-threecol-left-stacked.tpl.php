<?php
// $Id$

/**
 * @file panels_threecol_left_stacked.tpl.php
 * Template for a 3 column panel where the left and middle columns are
 * stacked with top and bottom rows.
 *
 * This template provides a three column 50%-25%-25% panel display layout, with
 * additional areas for the top and the bottom.
 *
 * Variables:
 * - $id: An optional CSS id to use for the layout.
 * - $content: An array of content, each item in the array is keyed to one
 *   panel of the layout. This layout supports the following sections:
 *   - $content['top']: Content in the top row.
 *   - $content['left']: Content in the left column.
 *   - $content['middle']: Content in the middle column.
 *   - $content['right']: Content in the right column.
 *   - $content['bottom']: Content in the bottom row.
 */
?>
<div class="panel-display panel-3col-left-stacked clear-block" <?php if (!empty($css_id)) { print "id=\"$css_id\""; } ?>>
  <div class="panel-col right">
    <div class="inside"><?php print $content['right']; ?></div>
  </div>
  <div class="left-wrapper">
    <div class="panel-col top">
      <div class="inside"><?php print $content['top']; ?></div>
    </div>
    <div class="panel-col left">
      <div class="inside"><?php print $content['left']; ?></div>
    </div>

    <div class="panel-col middle">
      <div class="inside"><?php print $content['middle']; ?></div>
    </div>
    <div class="panel-col bottom">
      <div class="inside"><?php print $content['bottom']; ?></div>
    </div>
  </div>
</div>

