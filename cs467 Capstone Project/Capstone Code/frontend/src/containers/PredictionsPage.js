import React from 'react';
import { render } from 'react-dom';
import { Link } from 'react-router-dom';
import { Breadcrumb, Container, Divider, Header, Icon, Loader, Table } from 'semantic-ui-react';
import _ from 'lodash'
import getJsonData from '../utils/getJsonData';

export default class PredictionsPage extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      sortColumn: 'symbol',
      sortDirection: 'ascending',
      stockData: [],
      isLoading: true
    };
  }

  async componentDidMount() {
    const stockData = await getJsonData('/stock-data/latest');

    this.setState({
      stockData: _.sortBy(stockData,
      [this.state.sortColumn]),
      isLoading: false
    });
  }

  handleSort = clickedColumn => () => {
    const { sortColumn, sortDirection, stockData } = this.state;

    if (sortColumn !== clickedColumn) {
      this.setState({
        sortColumn: clickedColumn,
        stockData: _.sortBy(stockData, [clickedColumn]),
        sortDirection: 'ascending',
      })

      return
    }

    this.setState({
      stockData: stockData.reverse(),
      sortDirection: sortDirection === 'ascending' ? 'descending' : 'ascending'
    })
  }

  render() {
    const { isLoading, sortColumn, sortDirection, stockData } = this.state;

    if (isLoading) {
      return (
        <Loader active />
      );
    } else if (stockData.length === 0) {
      return (
        <Container className='main'>
          <Header as='h2'>
            Load Failed
          </Header>
          <p>
            The data could not be loaded.
          </p>
        </Container>
      );
    }

    return (
      <Container className='main' style={{'overflowX': 'auto'}}>
        {/* Breadcrumb */}
        <Breadcrumb>
          <Breadcrumb.Section href='/'>
            Home
          </Breadcrumb.Section>
          <Breadcrumb.Divider />
          <Breadcrumb.Section active>
            Current Predictions
          </Breadcrumb.Section>
        </Breadcrumb>

        <Divider hidden />

        {/* Stock data table */}
        <Table sortable striped unstackable>
          {/* Header */}
          <Table.Header>
            <Table.Row>
              {/* Symbol */}
              <Table.HeaderCell
                sorted={sortColumn === 'symbol' ? sortDirection : null}
                onClick={this.handleSort('symbol')}
                width={2}
              >
                Symbol
              </Table.HeaderCell>

              {/* Name */}
              <Table.HeaderCell
                sorted={sortColumn === 'name' ? sortDirection : null}
                onClick={this.handleSort('name')}
                width={4}
              >
                Name
              </Table.HeaderCell>

              {/* 50-day prediction */}
              <Table.HeaderCell
                sorted={sortColumn === 'close' ? sortDirection : null}
                onClick={this.handleSort('close')}
                width={2}
                textAlign='right'
              >
                50-Day Prediction
              </Table.HeaderCell>

              {/* Change */}
              <Table.HeaderCell
                sorted={sortColumn === 'change' ? sortDirection : null}
                onClick={this.handleSort('change')}
                width={2}
                textAlign='right'
              >
                Change
              </Table.HeaderCell>

              {/* Percent change */}
              <Table.HeaderCell
                sorted={sortColumn === 'percent_change' ? sortDirection : null}
                onClick={this.handleSort('percent_change')}
                width={2}
                textAlign='right'
              >
                Percent Change
              </Table.HeaderCell>
            </Table.Row>
          </Table.Header>
          {/* Body */}
          <Table.Body>
            {Object.values(stockData).map((data) =>
              <Table.Row key={data.symbol}>
                {/* Symbol */}
                <Table.Cell key={data.symbol}>
                <Link to={{pathname: '/stocks/' + data.symbol, state: { prevPath: this.props.location.pathname }}}>
                  {data.symbol}
                </Link>
                </Table.Cell>

                {/* Name */}
                <Table.Cell key={data.name}>
                  {data.name}
                </Table.Cell>

                {/* Prediction */}
                <Table.Cell textAlign='right' key={`${data.symbol}-${data.close}-close`}>
                  N/A
                </Table.Cell>

                {/* Change */}
                <Table.Cell textAlign='right' key={`${data.symbol}-${data.change}-change`}>
                  N/A
                </Table.Cell>

                {/* Percent change */}
                <Table.Cell textAlign='right' key={`${data.symbol}-${data.percent_change}-percent-change`}>
                  N/A
                </Table.Cell>
              </Table.Row>
            )}
          </Table.Body>
        </Table>
      </Container>
    );
  }
}
