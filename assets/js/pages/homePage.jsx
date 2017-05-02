import React from 'react';
import ReactDOM from 'react-dom';
import HomePageContainer from '../containers/HomePageContainer';
import CardContainer from '../containers/card_container';

const page = 
<div>
	<HomePageContainer />
	<CardContainer />
</div>;

ReactDOM.render(page, document.getElementById('react-app'));
