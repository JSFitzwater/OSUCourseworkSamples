import React from 'react';
import { Container, Icon, List } from 'semantic-ui-react';

const SiteFooter = () => (
  <Container className='footer'>
    <List horizontal>
      <List.Item disabled>
        © {(new Date()).getFullYear()} Stock Market Forecaster
      </List.Item>
      <List.Item href='#'>
        <Icon name='github' />
      </List.Item>
    </List>
  </Container>
)

export default SiteFooter;
