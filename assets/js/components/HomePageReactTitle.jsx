import React, { PropTypes } from 'react';

const HomePageReactTitle = ({ title }) => {
  const homeURL = window.Django.url('home');

  return <h2>{title} (this is a page {homeURL})</h2>;
};

HomePageReactTitle.propTypes = {
  title: PropTypes.string.isRequired,
};

export default HomePageReactTitle;
