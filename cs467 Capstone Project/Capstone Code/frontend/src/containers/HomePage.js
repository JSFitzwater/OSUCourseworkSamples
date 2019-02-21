import React from 'react';
import { render } from 'react-dom';
import { Link } from 'react-router-dom';
import { Button, Container, Divider, Header, Grid, Image } from 'semantic-ui-react';

const HomePage = () => (
  <React.Fragment>
    {/* Masthead */}
    <Grid padded='horizontally' align='center' verticalAlign='middle' className='masthead' columns={1}>
      <Grid.Row color='teal'>
        <Grid.Column color='teal'>
          <Header as='h1' inverted>
            Evalulate future stock performance
          </Header>
          <Header as='h2' inverted>
            Make informed investing decisions today with Stock Market Forecaster
          </Header>
          <Divider hidden />
          <Button as={Link} to='/predictions' size='huge' inverted>
            View current predictions
          </Button>
        </Grid.Column>
      </Grid.Row>
    </Grid>

    {/* Content */}
    <Container className='main'>
      {/* Information panel #1 */}
      <Grid verticalAlign='middle' columns={2}>
        <Grid.Row>
          <Grid.Column align='center'>
            <Image src='/static/img/analytics-monitor.svg' size='large' />
          </Grid.Column>
          <Grid.Column>
            <Header as='h1' color='grey'>
              Stock market forecasts
            </Header>
            <Header as='h1' color='grey'>
              <Header.Subheader>
                Using Long Short-Term Memory (LSTM) models, we are able to provide simple 50-day stock price predictions.
              </Header.Subheader>
            </Header>
          </Grid.Column>
        </Grid.Row>
      </Grid>

      <Divider hidden />

      {/* Information panel #2 */}
      <Grid verticalAlign='middle' columns={2}>
        <Grid.Row>
          <Grid.Column>
            <Header as='h1' color='grey'>
              Stock market history
            </Header>
            <Header as='h1' color='grey'>
              <Header.Subheader>
                Stock prices are updated daily, and we have two years of historical data along with an interactive chart for each stock.
              </Header.Subheader>
            </Header>
            <Divider hidden />
            <Button as={Link} to='/prices' size='large'>
              See latest prices
            </Button>
          </Grid.Column>
          <Grid.Column align='center'>
            <Image src='/static/img/analytics-laptop.svg' size='large' />
          </Grid.Column>
        </Grid.Row>
      </Grid>
    </Container>
  </React.Fragment>
);

export default HomePage;
