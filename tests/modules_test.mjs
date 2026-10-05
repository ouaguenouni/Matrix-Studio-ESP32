import assert from 'node:assert/strict';
import {weatherKind, weatherLabel, formatTemperature, readForecast, readSearchPlace, readReversePlace} from '../web/weather.mjs';
import {CLOCK_THEMES, clockFace, digitWidth} from '../web/clock.mjs';
import {abbreviateCount, parseYoutubeChannel, youtubeSlide, youtubeChannelUrl, YOUTUBE_HANDLE} from '../web/youtube.mjs';

assert.equal(weatherKind(0), 'sun');
assert.equal(weatherKind(2), 'cloud');
assert.equal(weatherKind(45), 'fog');
assert.equal(weatherKind(61), 'rain');
assert.equal(weatherKind(71), 'snow');
assert.equal(weatherKind(95), 'storm');
assert.equal(weatherKind(3), 'cloud');
assert.equal(weatherLabel('rain'), 'Rain');
assert.throws(() => weatherKind(-1));
assert.equal(formatTemperature(18.6), '19');
assert.equal(formatTemperature(-0.4), '0');

const forecast = readForecast({current: {temperature_2m: 12.2, weather_code: 0}});
assert.equal(forecast.kind, 'sun');
assert.equal(forecast.temperature, 12.2);
assert.throws(() => readForecast({current: {temperature_2m: 1}}));
assert.deepEqual(readSearchPlace({results: [{name: 'Lyon', latitude: 45.75, longitude: 4.85}]}),
  {name: 'Lyon', latitude: 45.75, longitude: 4.85});
assert.throws(() => readSearchPlace({results: []}));
assert.equal(readReversePlace({city: 'Lyon'}), 'Lyon');
assert.equal(readReversePlace({locality: 'Old Town'}), 'Old Town');
assert.throws(() => readReversePlace({}));

const morning = clockFace(new Date(2026, 0, 2, 9, 5, 8));
assert.equal(morning.text, '09:05');
assert.equal(morning.showColon, true);
assert.equal(morning.bar, Math.max(1, Math.round((9 / 60) * 64)));
assert.equal(clockFace(new Date(2026, 0, 2, 9, 5, 7)).showColon, false);
assert.equal(digitWidth('09:05', 3) < 64, true);
assert.equal(CLOCK_THEMES.classic.blink, true);
assert.equal(CLOCK_THEMES.neon.foreground, '#00ffff');
assert.equal(CLOCK_THEMES.amber.background, '#2a1408');

assert.equal(abbreviateCount(0), '0');
assert.equal(abbreviateCount(999), '999');
assert.equal(abbreviateCount(1000), '1K');
assert.equal(abbreviateCount(1200), '1.2K');
assert.equal(abbreviateCount(999999), '1M');
assert.equal(abbreviateCount(3400000), '3.4M');
assert.equal(abbreviateCount(1500000000), '1.5B');
assert.throws(() => abbreviateCount(-1));

const channel = parseYoutubeChannel({items: [{
  snippet: {title: 'Cosmic Hippo Sounds'},
  statistics: {subscriberCount: '1200', viewCount: '3400000', videoCount: '42'},
}]});
assert.equal(channel.title, 'Cosmic Hippo Sounds');
assert.deepEqual(youtubeSlide(channel, 0), {title: 'Cosmic Hippo Sounds', label: 'SUBS', value: '1.2K'});
assert.deepEqual(youtubeSlide(channel, 1), {title: 'Cosmic Hippo Sounds', label: 'VIEWS', value: '3.4M'});
assert.equal(youtubeSlide(channel, 3).label, 'SUBS');
const hidden = parseYoutubeChannel({items: [{
  statistics: {hiddenSubscriberCount: true, viewCount: '10', videoCount: '1'},
}]});
assert.equal(hidden.subscribers, null);
assert.equal(youtubeSlide(hidden, 0).value, '--');
assert.throws(() => parseYoutubeChannel({items: []}));
const url = new URL(youtubeChannelUrl('test-key'));
assert.equal(url.searchParams.get('forHandle'), YOUTUBE_HANDLE);
assert.equal(url.searchParams.get('part'), 'statistics,snippet');
assert.equal(url.searchParams.get('key'), 'test-key');
assert.throws(() => youtubeChannelUrl('  '));

console.log('Modules: weather codes, clock face, and YouTube counts passed.');
