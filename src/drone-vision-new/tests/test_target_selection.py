import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from target_selection import Detection, Selection, choose_target

class SelectionTests(unittest.TestCase):
    def test_rectangle_distance_precedes_center_distance(self):
        inside = Detection((0, 0, 100, 100), .6, 'large')
        outside = Detection((51, 49, 55, 51), .9, 'near')
        self.assertEqual(choose_target([outside, inside], (.5, .5, 0), (100, 100)), inside)

    def test_center_and_confidence_ties(self):
        low = Detection((40, 40, 60, 60), .6, 'low')
        high = Detection((40, 40, 60, 60), .9, 'high')
        farther = Detection((0, 0, 90, 90), 1, 'farther')
        self.assertEqual(choose_target([low, farther, high], (.5, .5, 0), (100, 100)), high)

    def test_actual_frame_dimensions_and_radius(self):
        item = Detection((900, 400, 1000, 500), .8, 'item')
        self.assertEqual(choose_target([item], (.95, .45, 0), (1000, 1000)), item)
        self.assertIsNone(choose_target([item], (0, 0, .01), (1000, 1000)))

    def test_invalid_and_empty_inputs(self):
        self.assertIsNone(choose_target([], (.5, .5, 0), (1920, 1080)))
        for point in ((float('nan'), .5, 0), (1.1, .5, 0), (.5, .5, -1)):
            with self.assertRaises(ValueError): choose_target([], point, (100, 100))
        with self.assertRaises(ValueError): choose_target([], (.5, .5, 0), (0, 100))

    def test_radius_is_width_normalized_on_tall_and_wide_frames(self):
        # Distance 12 pixels exceeds radius .1 * width100, even though the
        # old diagonal normalization would include this target on a tall frame.
        item = Detection((62, 500, 70, 510), .9, 'outside')
        self.assertIsNone(choose_target([item], (.5, .5, .1), (100, 1000)))
        near = Detection((58, 500, 60, 510), .8, 'near')
        self.assertEqual(choose_target([item, near], (.5, .5, .1), (100, 1000)), near)
        self.assertIsNone(choose_target([item], (.5, .5, 0), (100, 1000)))

    def test_status_tracks_current_bbox_and_lost_detection(self):
        state = Selection(ttl_seconds=2)
        state.set_point(.5, .5, .1, now=10, generation=7)
        item = Detection((40, 40, 60, 60), .8, 'item')
        selected, status = state.evaluate([item], (100, 100), now=11)
        self.assertEqual(selected, item)
        self.assertEqual(status['generation'], 7)
        self.assertEqual(status['bbox'], [.4, .4, .6, .6])
        self.assertIsNone(state.evaluate([], (100, 100), now=11)[1]['bbox'])
        self.assertIsNone(state.evaluate([item], (100, 100), now=13)[1]['bbox'])
        state.set_point(.5, .5, .1, now=20, generation=8)
        state.clear(generation=9)
        self.assertIsNone(state.evaluate([item], (100, 100), now=20)[0])

    def test_selection_expiry_disconnect_and_detection_reordering(self):
        state = Selection(ttl_seconds=2)
        state.set_point(.5, .5, 0, now=10)
        a = Detection((40, 40, 60, 60), .8, 'a')
        b = Detection((80, 80, 90, 90), .9, 'b')
        self.assertEqual(state.choose([a, b], (100, 100), now=11), a)
        self.assertEqual(state.choose([b, a], (100, 100), now=11), a)
        self.assertIsNone(state.choose([a], (100, 100), now=13))
        state.set_point(.5, .5, 0, now=20); state.clear()
        self.assertIsNone(state.choose([a], (100, 100), now=20))

if __name__ == '__main__': unittest.main()
