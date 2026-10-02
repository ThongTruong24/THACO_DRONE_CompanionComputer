import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from bbox_renderer import render_plan
from target_selection import Detection

class RendererTests(unittest.TestCase):
    def test_selection_style_and_no_index_identity(self):
        a=Detection((1, 2, 30, 40), .8, 'person')
        b=Detection((50, 60, 70, 80), .9, 'car')
        normal, selected=render_plan([b, a], a)
        self.assertEqual(normal.color, (0, 255, 0))
        self.assertEqual(selected.color, (0, 255, 255))
        self.assertGreater(selected.thickness, normal.thickness)
        self.assertIn('[SELECTED]', selected.label)
        self.assertNotIn('[SELECTED]', normal.label)

    def test_cleared_selection(self):
        item=Detection((1, 2, 30, 40), .8, 'person')
        self.assertEqual(render_plan([item], None)[0].color, (0, 255, 0))

if __name__ == '__main__': unittest.main()
