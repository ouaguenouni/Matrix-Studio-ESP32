export const YOUTUBE_HANDLE = 'CosmicHippoSounds';

export function abbreviateCount(value) {
  const rounded = Math.round(Number(value));
  if (!Number.isFinite(rounded) || rounded < 0) throw new Error('Invalid count');
  const units = ['', 'K', 'M', 'B'];
  let number = rounded;
  let unit = 0;
  while (number >= 999.5 && unit < units.length - 1) {
    number /= 1000;
    unit += 1;
  }
  const digits = unit === 0 || number >= 100 || Number.isInteger(number)
    ? number.toFixed(0)
    : number.toFixed(1);
  return digits.replace(/\.0$/, '') + units[unit];
}

export function parseYoutubeChannel(data) {
  const item = data?.items?.[0];
  const stats = item?.statistics;
  if (!item || !stats) throw new Error('That channel could not be found.');
  const views = Number(stats.viewCount);
  const videos = Number(stats.videoCount);
  if (!Number.isFinite(views) || !Number.isFinite(videos)) throw new Error('YouTube returned an unexpected response.');
  const hidden = stats.hiddenSubscriberCount === true;
  const subscribers = hidden ? null : Number(stats.subscriberCount);
  if (!hidden && !Number.isFinite(subscribers)) throw new Error('YouTube returned an unexpected response.');
  return {
    title: String(item.snippet?.title || 'Cosmic Hippo Sounds'),
    subscribers,
    views,
    videos,
  };
}

export function youtubeSlide(channel, index) {
  const slides = [
    ['SUBS', channel.subscribers],
    ['VIEWS', channel.views],
    ['VIDS', channel.videos],
  ];
  const [label, value] = slides[((index % slides.length) + slides.length) % slides.length];
  return {title: channel.title, label, value: value === null ? '--' : abbreviateCount(value)};
}

export function youtubeChannelUrl(key) {
  const trimmed = String(key || '').trim();
  if (!trimmed) throw new Error('Add a YouTube Data API key first.');
  const params = new URLSearchParams({
    part: 'statistics,snippet',
    forHandle: YOUTUBE_HANDLE,
    key: trimmed,
  });
  return `https://www.googleapis.com/youtube/v3/channels?${params}`;
}
