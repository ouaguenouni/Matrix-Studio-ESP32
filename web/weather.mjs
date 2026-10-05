export function weatherKind(code) {
  const value = Number(code);
  if (!Number.isInteger(value) || value < 0) throw new Error('Unexpected weather code');
  if (value === 0) return 'sun';
  if (value <= 3) return 'cloud';
  if (value === 45 || value === 48) return 'fog';
  if ((value >= 51 && value <= 67) || (value >= 80 && value <= 82)) return 'rain';
  if ((value >= 71 && value <= 77) || value === 85 || value === 86) return 'snow';
  if (value >= 95 && value <= 99) return 'storm';
  return 'cloud';
}

export function weatherLabel(kind) {
  return {sun: 'Clear', cloud: 'Cloud', fog: 'Fog', rain: 'Rain', snow: 'Snow', storm: 'Storm'}[kind] || 'Cloud';
}

export function formatTemperature(celsius) {
  if (!Number.isFinite(celsius)) throw new Error('Unexpected temperature');
  return String(Math.round(celsius));
}

export function readForecast(data) {
  const current = data?.current;
  if (!current || !Number.isFinite(current.temperature_2m) || !Number.isInteger(current.weather_code))
    throw new Error('Weather service returned an unexpected response.');
  const kind = weatherKind(current.weather_code);
  return {temperature: current.temperature_2m, code: current.weather_code, kind};
}

export function readSearchPlace(data) {
  const hit = data?.results?.[0];
  if (!hit || !hit.name || !Number.isFinite(hit.latitude) || !Number.isFinite(hit.longitude))
    throw new Error('No matching city.');
  return {name: String(hit.name), latitude: hit.latitude, longitude: hit.longitude};
}

export function readReversePlace(data) {
  const name = data?.city || data?.locality || data?.principalSubdivision;
  if (!name) throw new Error('Could not name this location.');
  return String(name);
}
