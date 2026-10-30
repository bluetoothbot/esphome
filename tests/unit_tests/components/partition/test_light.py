"""Tests for the partition light segment validation."""

import pytest

from esphome.components.partition.light import MAX_PARTITION_LEDS, validate_total_length
import esphome.config_validation as cv
from esphome.const import CONF_FROM, CONF_ID, CONF_TO
from esphome.types import ConfigType


def _segment(from_: int, to: int) -> ConfigType:
    return {CONF_ID: "strip", CONF_FROM: from_, CONF_TO: to}


def test_total_length_at_limit_is_accepted() -> None:
    segments = [_segment(0, MAX_PARTITION_LEDS - 1)]
    assert validate_total_length(segments) == segments


def test_total_length_counts_single_light_segments() -> None:
    segments = [_segment(0, MAX_PARTITION_LEDS - 2), {"single_light_id": "rgb"}]
    assert validate_total_length(segments) == segments


@pytest.mark.parametrize(
    "segments",
    [
        [_segment(0, 65535)],
        [_segment(0, 40000), _segment(0, 40000)],
        [_segment(0, MAX_PARTITION_LEDS - 1), {"single_light_id": "rgb"}],
    ],
)
def test_total_length_over_limit_is_rejected(segments: list[ConfigType]) -> None:
    with pytest.raises(cv.Invalid, match="at most 65535"):
        validate_total_length(segments)
