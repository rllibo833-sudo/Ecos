"""Dependency-free checks for ECOS's conceptual spatial contract.

Run with: python -m unittest discover -s tests -p 'test_*.py'
These checks validate declared lore relationships, not real map geometry.
"""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "world/offline-10k-ha/spatial-contract.json"


class WorldContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads(CONTRACT.read_text(encoding="utf-8"))

    def test_area_accounting(self):
        area = self.data["area"]
        self.assertEqual(area["value"], 10_000)
        self.assertEqual(area["equivalent_km2"], 100)
        areas = {item["id"]: item for item in self.data["subareas"]}
        self.assertEqual(areas["mosque_area"]["area_hectare"], 1_000)
        self.assertEqual(areas["outside_mosque_area"]["area_hectare"], 9_000)
        self.assertEqual(
            areas["mosque_area"]["area_hectare"]
            + areas["outside_mosque_area"]["area_hectare"],
            area["value"],
        )

    def test_mosque_area_is_inside_offline_core(self):
        mosque = next(item for item in self.data["subareas"] if item["id"] == "mosque_area")
        self.assertEqual(mosque["relation"], "contained_within_offline_core")
        self.assertEqual(mosque["province"], "Arunika")
        self.assertEqual(mosque["geometry_status"], "not_defined")

    def test_juailin_location_is_not_invented(self):
        juailin = next(item for item in self.data["places"] if item["id"] == "juailin")
        self.assertEqual(juailin["province"], "Arunika")
        self.assertIsNone(juailin["coordinates"])
        self.assertIsNone(juailin["exact_corner"])
        self.assertEqual(juailin["geometry_status"], "not_defined")

    def test_contract_does_not_claim_final_geometry(self):
        self.assertEqual(self.data["status"], "conceptual_spatial_contract")
        self.assertIn("offline_core_polygon", self.data["unknowns"])
        self.assertIn("roads_and_routes", self.data["unknowns"])


if __name__ == "__main__":
    unittest.main()
